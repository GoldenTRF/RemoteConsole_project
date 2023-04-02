#pragma once

#include "IpValidator.h"
#include "LocalAddrValidator.h"
#include "../../proto/RemoteConsole.grpc.pb.h"

#include <memory>
#include <conio.h>
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <vector>



class GrpcClient; // Forward declaration of the client class

namespace utils
{
	void errorExit();
	bool connectionAlive(std::shared_ptr<GrpcClient>);
	std::string getAddress();
	std::vector<std::string> strSplit(const std::string&);
}