#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>

#include <windows.h>
#include <stdio.h>

#include "CommonFunc.h"
#include "Services.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;



namespace ServerSpace
{
    static void consoleBufferReading()
    {
        // Make console buffer vector
        CONSOLE_SCREEN_BUFFER_INFO bufferInfo{};

        do
        {
            if (!ServerHelperSpace::isClientConnected)
            {
                continue;
            }

            Sleep(100);
            if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufferInfo)) {
                std::cerr << "Failed to get console screen buffer info" << std::endl;
                return;
            } // Update the buffer information
            
            std::lock_guard<std::mutex> lock(ServerHelperSpace::bufferMutex);

            COORD bufferSize = bufferInfo.dwSize;
            SMALL_RECT smal = { 0, 0, bufferInfo.dwSize.X, bufferInfo.dwSize.Y }; // The visable area to reading

            std::vector<CHAR_INFO> currentBuffer(bufferSize.X * bufferSize.Y);
            ServerHelperSpace::prevConsoleBuffer = ServerHelperSpace::currentConsoleBuffer; // Swap buffers to update

            ReadConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE), currentBuffer.data(), bufferSize, { 0,0 }, &smal);

            ServerHelperSpace::currentConsoleBuffer = currentBuffer; // Update the new console buffer

            // Update the console cursor position
            ServerHelperSpace::cursorPosition.X = bufferInfo.dwCursorPosition.X;
            ServerHelperSpace::cursorPosition.Y = bufferInfo.dwCursorPosition.Y;
        } while (true);

        return;
    }

    static void RunServer()
    {
        std::string serverAddress("0.0.0.0:50051");

        // Implemented services
        AuthorizationServiceImpl   authorizationSer;
        EventDetectionServiceImpl  eventDetectSer;
        ConsoleControllServiceImpl consoleControllSer;

        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        grpc::ServerBuilder builder;

        grpc::SslServerCredentialsOptions::PemKeyCertPair pemKeyCert;
        grpc::SslServerCredentialsOptions creds;

        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&authorizationSer);
        builder.RegisterService(&eventDetectSer);
        builder.RegisterService(&consoleControllSer);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

        // Read the console buffer of the server console screen in other thread
        std::thread consoleReadingTh(ServerSpace::consoleBufferReading);
        consoleReadingTh.detach();

        // Server waits for the client requests
        server->Wait();
    }
}