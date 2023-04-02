#include "client.h"
#include "UtilFuncs.h"



bool GrpcClient::checkBuffer()
{
    grpc::ClientContext context;
    const RemoteConsole::Empty request;
    RemoteConsole::Empty reply;

    grpc::Status changeStatus = ConsoleStub_->CheckBuffer(&context, request, &reply);
    if (!changeStatus.ok())
    {
        return false; // This is the same buffer that last time was there
    }
    return true; // The buffer has been changed
}

bool GrpcClient::connectionAlive()
{
    grpc::ClientContext context;
    const RemoteConsole::Empty request;
    RemoteConsole::Empty reply;

    grpc::Status status = EventStub_->connectionAlive(&context, request, &reply);
    if (!status.ok())
        return false;

    return true;
}

COORD GrpcClient::getConsoleCursor()
{
    grpc::ClientContext context;
    RemoteConsole::CursorMess cursorReply;
    RemoteConsole::Empty request;

    grpc::Status status = ConsoleStub_->GetCirsorPosition(&context, request, &cursorReply);

    if (!status.ok())
        return COORD{ 0, 0 };

    return COORD{ (SHORT)cursorReply.cursorx(), (SHORT)cursorReply.cursory() };
}

bool GrpcClient::getGrpcConsoleBuffer(std::vector<CHAR_INFO>& bufferReply)
{
    grpc::ClientContext context;
    const RemoteConsole::Empty request;

    std::unique_ptr<grpc::ClientReader<RemoteConsole::CharChank>> reader(ConsoleStub_->SendCharInfo(&context, request));

    std::vector<char> vectorFirst;
    RemoteConsole::CharChank chankMess;

    while (reader->Read(&chankMess))
    {
        vectorFirst.insert(vectorFirst.end(), chankMess.chunk().begin(), chankMess.chunk().end());
    }

    grpc::Status readerStatus = reader->Finish();

    if (!readerStatus.ok())
    {
        return false;
    }

    bufferReply.reserve(vectorFirst.size() / sizeof(CHAR_INFO));
    memcpy(bufferReply.data(), vectorFirst.data(), vectorFirst.size());
    return true;
}

void GrpcClient::sendKeyEvent(const INPUT_RECORD& input)
{
    grpc::ClientContext context;
    RemoteConsole::KeyEventMess clientReq;
    RemoteConsole::Empty serverRep;
    
    clientReq.set_keydown(input.Event.KeyEvent.bKeyDown);
    clientReq.set_controlkeystate(input.Event.KeyEvent.dwControlKeyState);
    clientReq.set_ascii_char(input.Event.KeyEvent.uChar.AsciiChar);
    clientReq.set_unicode_char(input.Event.KeyEvent.uChar.UnicodeChar);
    clientReq.set_virtualkeycode(input.Event.KeyEvent.wVirtualKeyCode);
    clientReq.set_virtualscancode(input.Event.KeyEvent.wVirtualScanCode);
    clientReq.set_wrepcount(input.Event.KeyEvent.wRepeatCount);

    grpc::Status status = EventStub_->keyDetection(&context, clientReq, &serverRep);
}

void GrpcClient::sendMouseEvent(const INPUT_RECORD& input)
{
    grpc::ClientContext context;
    RemoteConsole::MouseEventMess clientReq;
    RemoteConsole::Empty serverRep;

    clientReq.set_buttonstate(input.Event.MouseEvent.dwButtonState);
    clientReq.set_controlkeystate(input.Event.MouseEvent.dwControlKeyState);
    clientReq.set_eventflag(input.Event.MouseEvent.dwEventFlags);
    clientReq.set_mousex(input.Event.MouseEvent.dwMousePosition.X);
    clientReq.set_mousey(input.Event.MouseEvent.dwMousePosition.Y);

    grpc::Status status = EventStub_->mouseDetection(&context, clientReq, &serverRep);
}

void GrpcClient::clientExitDetection()
{
    grpc::ClientContext context;
    RemoteConsole::Empty clientReq;
    RemoteConsole::Empty serverRep;

    grpc::Status status = EventStub_->clientDisconnection(&context, clientReq, &serverRep);
}

std::pair<int, std::string> GrpcClient::primaryConnect()
{
    grpc::ClientContext context;
    RemoteConsole::Empty clientRequest;
    RemoteConsole::AuthorizationReply serverReply;

    grpc::Status status = AuthStub_->primaryConnection(&context, clientRequest, &serverReply);

    if (status.error_code() == grpc::StatusCode::OUT_OF_RANGE)
        return std::move(std::pair<int, std::string>(-1, "The server is busy at the moment!\n"));
    if (!status.ok())
        return std::move(std::pair<int, std::string>(-2, "Can not find the server! Is the server running?\n"));

    return std::move(std::pair<int, std::string>(1, "Successed connection!\n"));
}

grpc::Status GrpcClient::runAuthorizationService()
{
    grpc::ClientContext clientCtx;
    RemoteConsole::AuthorizationRequest clientAuthReq;
    RemoteConsole::AuthorizationReply serverAuthReply;

    std::string serverLogin;
    std::string serverPass;

    std::cout << "Enter the server login: ";
    std::cin >> serverLogin;
    std::cout << "Enter the server password: ";
    std::cin >> serverPass;

    std::cout << "\x1B[2J\x1B[H";

    clientAuthReq.set_clientlogin(serverLogin);
    clientAuthReq.set_clientpassword(serverPass);

    grpc::Status status = AuthStub_->clientAuthorization(&clientCtx, clientAuthReq, &serverAuthReply);

    if (status.ok())
        std::cout << "Your authowization has been confirmed!\n";
    else
        std::cout << "Your connection is not approved!Ivalid data.\n";
                
    return status;
}

GrpcClient::~GrpcClient()
{
}