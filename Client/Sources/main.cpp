#include <iostream>
#include <memory>
#include <fstream>
#include <utility>
#include <thread>

#include "client.h"
#include "UtilFuncs.h"  // Helpful function for client only 
#include "CommonFunc.h" // Lib for client and server



namespace ClientHelperSpace
{
    std::shared_ptr<GrpcClient> client;             // Our client object
    
    static DWORD oldConsoleMode{};                  // The Console mode we have with application running
    static COORD prevCursorPos{};                   // The remembered cursor position to compiration
    static COORD currentCursorPos{};                // The current cursor position we get from server
    static CONSOLE_SCREEN_BUFFER_INFO bufferInfo{}; // Struct that have information about console buffer of client app
}


// 1. The verification of server address and process port
// 2. The verification of the user log in information
void clientRegistration()
{
    // Verification of server address and process port
    bool status = false;
    while (!status) 
    {
        // Validation of the address client input
        std::string userAddressInput = utils::getAddress(); 

        // Connect the client with the entered address and port
        ClientHelperSpace::client.reset(new GrpcClient(grpc::CreateChannel(userAddressInput, grpc::InsecureChannelCredentials()))); 

        system("cls");

        // The pair has bool server reply flag and string message
        std::pair<bool, std::string> serverReply(ClientHelperSpace::client->primaryConnect());
        std::cout << serverReply.second; // Show the server reply message

        // Check connection with server
        if (serverReply.first) 
            break;
    }

    // Verification of the user log in information
    status = false;
    while (!status) 
    {
        // Check if the entered information is correct
        if ((ClientHelperSpace::client->runAuthorizationService()).ok())
        {
            break;
        }
    }
}

// Recieve and write console buffer 
void writeConsoleBuffer()
{
    // Buffer thst hold the char elements of the server buffer 
    std::vector<CHAR_INFO> clientBufferVector;
    
    //Check that the buffer is changed and buffer is successful accepted from server
    if (ClientHelperSpace::client->checkBuffer() && ClientHelperSpace::client->getGrpcConsoleBuffer(clientBufferVector))
    {
        SMALL_RECT smal = { 0, 0, ClientHelperSpace::bufferInfo.dwSize.X, ClientHelperSpace::bufferInfo.dwSize.Y }; // Visible area for writting
        WriteConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE), clientBufferVector.data(), ClientHelperSpace::bufferInfo.dwSize, { 0,0 }, &smal);

        ClientHelperSpace::currentCursorPos = ClientHelperSpace::client->getConsoleCursor();

        // If cursors position is not the same
        if (!common::compareCursor(ClientHelperSpace::currentCursorPos, ClientHelperSpace::prevCursorPos))
        {
            ClientHelperSpace::prevCursorPos = ClientHelperSpace::currentCursorPos;
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), ClientHelperSpace::currentCursorPos);
        }
    }
    return;
}

// Function to execute when close button is clicked
BOOL WINAPI ConsoleExitDetect(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_CLOSE_EVENT)
    {
        // Send to the server disconnect message
        ClientHelperSpace::client->clientExitDetection();
        return TRUE;
    }

    return FALSE;
}

// Mouse and key event detection and sending to the server 
void eventDetection()
{
    DWORD eventAmount{};
    std::vector<INPUT_RECORD> eventBuffer(128);

    while (1)
    {
        // Allow the mouse input in the console and read events
        bool setModeReply =  SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), eventBuffer.data(), eventBuffer.size(), &eventAmount);

        for (auto eventCurrent = 0; eventCurrent < eventAmount; eventCurrent++)
        {
            // Send to the server event information
            switch (eventBuffer.at(eventCurrent).EventType)
            {
            case KEY_EVENT: 
                ClientHelperSpace::client->sendKeyEvent(eventBuffer.at(eventCurrent));
                break;

            case MOUSE_EVENT:
                ClientHelperSpace::client->sendMouseEvent(eventBuffer.at(eventCurrent));
                break;
            }
        }
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ClientHelperSpace::oldConsoleMode);
    }
}

int main(int argc, char** argv)
{
    clientRegistration();

    // Set the callbeck function to executing when close button is clicked 
    SetConsoleCtrlHandler(ConsoleExitDetect, TRUE);

    // Make thread to mouse and key event detaction
    std::thread eventDetectThread(eventDetection);
    eventDetectThread.detach();

    system("cls");

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ClientHelperSpace::bufferInfo);

    // Run the writing the recieved console buffer
    while (utils::connectionAlive(ClientHelperSpace::client))
    {
        writeConsoleBuffer();
    }

    // If the connection with server is lost
    utils::erExit();
}