#pragma once

#include <grpcpp/grpcpp.h>
#include <fstream>

#include "../../proto/RemoteConsole.grpc.pb.h"



class GrpcClient
{
public:
    GrpcClient(std::shared_ptr<grpc::Channel> channel)
        : AuthStub_(RemoteConsole::AuthorizationService::NewStub(channel))
        , ConsoleStub_(RemoteConsole::ConsoleControllService::NewStub(channel))
        , EventStub_(RemoteConsole::EventDetectionService::NewStub(channel))
    {}

    // Client connection operations
    grpc::Status runAuthorizationService();
    std::pair<int, std::string> primaryConnect();

    // Client check operations
    bool checkBuffer();
    bool connectionAlive();

    // Client getter operations
    bool getGrpcConsoleBuffer(std::vector<CHAR_INFO>&);
    COORD getConsoleCursor();
    
    // Client events detection operations
    void sendKeyEvent(const INPUT_RECORD&);
    void sendMouseEvent(const INPUT_RECORD&);
    void clientExitDetection();

    ~GrpcClient();

private:
    // Stubs for communication with server
    std::unique_ptr<RemoteConsole::AuthorizationService::Stub>      AuthStub_;
    std::unique_ptr<RemoteConsole::ConsoleControllService::Stub>    ConsoleStub_;
    std::unique_ptr<RemoteConsole::EventDetectionService::Stub>     EventStub_;
};