cmake_minimum_required (VERSION 3.8)

project(Client)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

set(TOOLSPATH "../Tools")
include_directories("${TOOLSPATH}/Include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Headers")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Sources")


set(CPPHEADERS	"Headers/client.h"
				"Headers/UtilFuncs.h")

set(CPPSOURCES  "Sources/main.cpp"
				"Sources/client.cpp"
				"Sources/UtilFuncs.cpp")

add_executable (${PROJECT_NAME} ${CPPSOURCES} ${CPPHEADERS} ../proto/RemoteConsole.grpc.pb.cc ../proto/RemoteConsole.pb.cc)

target_link_libraries(${PROJECT_NAME} PRIVATE gRPC::grpc++ gRPC::grpc++_reflection gRPC::gpr gRPC::grpc gRPC::grpc++ protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite Tools_lib::tools)
