#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "../../proto/RemoteConsole.grpc.pb.h"

const int CHANK_SIZE = 64 * 1024;

// namespace`s variables
namespace ServerHelperSpace
{
    static std::vector<CHAR_INFO> currentConsoleBuffer{};
    static std::vector<CHAR_INFO> prevConsoleBuffer{};
    static int firstShow = 0; // Flag of transfer times
    static bool isClientConnected = false;

    std::mutex bufferMutex;

    static COORD cursorPosition{};
    static PROCESS_INFORMATION serverProcessInfo;
}


// namespace`s functions
namespace ServerHelperSpace
{
    static void createServerProcess()
    {
        STARTUPINFO si = { sizeof(STARTUPINFO) };
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&serverProcessInfo, sizeof(serverProcessInfo));

        // Child process creating
        CreateProcess(std::string("C:\\Windows\\System32\\cmd.exe").c_str(), NULL, NULL, NULL, 0, NULL, NULL, NULL, &si, &serverProcessInfo);

        CloseHandle(serverProcessInfo.hThread);
        Sleep(500);

        // Free parent`s console to attach
        FreeConsole();

        // Attach child process console to parent console
        if (!AttachConsole(serverProcessInfo.dwProcessId)) {
            std::cerr << "Failed to attach to parent process console" << std::endl;
            return;
        }
    }

    static void closeServerProcess()
    {
        TerminateProcess(serverProcessInfo.hProcess, 0);
        system("cls");
    }
}



class EventDetectionServiceImpl final : public RemoteConsole::EventDetectionService::Service
{
    grpc::Status keyDetection(grpc::ServerContext* ctx
                            , const RemoteConsole::KeyEventMess* keyPressed
                            , RemoteConsole::Empty* serverRep)
    {
        INPUT_RECORD input{};
        DWORD eventWritten{};

        input.EventType = KEY_EVENT;
        input.Event.KeyEvent.bKeyDown = keyPressed->keydown();
        input.Event.KeyEvent.dwControlKeyState = keyPressed->controlkeystate();
        input.Event.KeyEvent.uChar.AsciiChar = keyPressed->ascii_char();
        input.Event.KeyEvent.uChar.UnicodeChar = keyPressed->unicode_char();
        input.Event.KeyEvent.wRepeatCount = keyPressed->wrepcount();
        input.Event.KeyEvent.wVirtualKeyCode = keyPressed->virtualkeycode();
        input.Event.KeyEvent.wVirtualScanCode = keyPressed->virtualscancode();

        WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &eventWritten);

        return grpc::Status::OK;
    }

    grpc::Status mouseDetection(grpc::ServerContext* ctx
                               , const RemoteConsole::MouseEventMess* mousePressed
                               , RemoteConsole::Empty* serverRep)
    {
        INPUT_RECORD input{};
        DWORD eventWritten{};

        input.EventType = MOUSE_EVENT;
        input.Event.MouseEvent.dwButtonState = mousePressed->buttonstate();
        input.Event.MouseEvent.dwControlKeyState = mousePressed->controlkeystate();
        input.Event.MouseEvent.dwEventFlags = mousePressed->eventflag();
        input.Event.MouseEvent.dwMousePosition.X = mousePressed->mousex();
        input.Event.MouseEvent.dwMousePosition.Y= mousePressed->mousey();

        WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &eventWritten);

        return grpc::Status::OK;
    }

    grpc::Status clientDisconnection(grpc::ServerContext* context
                                    , const RemoteConsole::Empty* request
                                    , RemoteConsole::Empty* response) override
    {
        ServerHelperSpace::closeServerProcess();
        ServerHelperSpace::isClientConnected = false;
        ServerHelperSpace::currentConsoleBuffer.clear();

        return grpc::Status(grpc::StatusCode::OK, "Client has been disconnected.");
    }

    grpc::Status connectionAlive(grpc::ServerContext* context
                                , const RemoteConsole::Empty* request
                                , RemoteConsole::Empty* response) override
    {
        return grpc::Status(grpc::StatusCode::OK, "Connection is alive.");
    }
};



class ConsoleControllServiceImpl final : public RemoteConsole::ConsoleControllService::Service
{
    grpc::Status GetCirsorPosition(grpc::ServerContext* context
                                   , const RemoteConsole::Empty* request
                                   , RemoteConsole::CursorMess* response) override
    {
        response->set_cursorx(ServerHelperSpace::cursorPosition.X);
        response->set_cursory(ServerHelperSpace::cursorPosition.Y);
        return grpc::Status(grpc::StatusCode::OK, "Cursor position is filled");
    }

    grpc::Status SendCharInfo(grpc::ServerContext* context
                              , const RemoteConsole::Empty* request
                              , grpc::ServerWriter<RemoteConsole::CharChank>* writer) override
    {
        std::vector<CHAR_INFO> bufferVector;
        {
            std::lock_guard<std::mutex> lock(ServerHelperSpace::bufferMutex);
            bufferVector = ServerHelperSpace::currentConsoleBuffer; // Get the last console buffer
        }
 
        int byteSended = 0;
        int byteToSend = bufferVector.size() * sizeof(CHAR_INFO);
        const char* currentByte = reinterpret_cast<char*>(bufferVector.data());

        RemoteConsole::CharChank chankMess;

        while (byteSended < byteToSend)
        {
            if (byteSended + CHANK_SIZE > byteToSend)
                chankMess.set_chunk(currentByte + byteSended, byteToSend - byteSended);

            else
                chankMess.set_chunk(currentByte + byteSended, CHANK_SIZE);

            if (!writer->Write(chankMess))
                return grpc::Status(grpc::StatusCode::CANCELLED, "Error writing buffer chank");

            byteSended += CHANK_SIZE;
        }

        return grpc::Status(grpc::StatusCode::OK, "The buffer is filled");
    }

    grpc::Status CheckBuffer(grpc::ServerContext* context
                            , const RemoteConsole::Empty* request
                            , RemoteConsole::Empty* response) override
    {
        if (!ServerHelperSpace::firstShow)
        {
            ServerHelperSpace::firstShow++;

            {
                std::lock_guard<std::mutex> lock(ServerHelperSpace::bufferMutex);
                // If there are two the same buffers (nothing has been changed)
                if (common::vBuffersCompare(ServerHelperSpace::currentConsoleBuffer, ServerHelperSpace::prevConsoleBuffer))
                {
                    return grpc::Status(grpc::StatusCode::CANCELLED, "The buffer is not changed");
                }
            }
        }
        return grpc::Status(grpc::StatusCode::OK, "The buffer is changed");
    }
};



class AuthorizationServiceImpl final : public RemoteConsole::AuthorizationService::Service
{
    grpc::Status primaryConnection(grpc::ServerContext* context
                                   , const RemoteConsole::Empty* request
                                   , RemoteConsole::AuthorizationReply* response) override
    {
        if (ServerHelperSpace::isClientConnected)
            return grpc::Status(grpc::StatusCode::OUT_OF_RANGE, "The client connection is not successed!");

        ServerHelperSpace::isClientConnected = true;
        return grpc::Status(grpc::StatusCode::OK, "The client connection is successed!");
    }

    grpc::Status clientAuthorization(grpc::ServerContext* ctx
                                    , const RemoteConsole::AuthorizationRequest* clientAuthReq
                                    , RemoteConsole::AuthorizationReply* serverAuthRep)
    {
        HANDLE hToken = 0;

        // Check the login data recieved from the client
        bool serverRep = LogonUserA(clientAuthReq->clientlogin().c_str(),
            NULL,
            clientAuthReq->clientpassword().c_str(),
            LOGON32_LOGON_NETWORK,
            LOGON32_PROVIDER_DEFAULT,
            &hToken);

        serverAuthRep->set_issuccessful(serverRep);

        if (!serverRep)
        {
            return grpc::Status(grpc::StatusCode::CANCELLED, "The connection is not approved!");
        }

        ServerHelperSpace::createServerProcess();
        return grpc::Status(grpc::StatusCode::OK, "The connection is successful!");
    }
};