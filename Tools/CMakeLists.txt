cmake_minimum_required (VERSION 3.8)

project(Tools)

include_directories("Include/")
include_directories("${PROJECT_NAME}")

set(CPPSOURCES	"Sources/IpValidator.cpp"
				"Sources/LocalAddrValidator.cpp"
				"Sources/CommonFunc.cpp")

set(CPPHEADERS	"Include/ValidatorBaseClass.h"
				"Include/IpValidator.h"
				"Include/LocalAddrValidator.h"
				"Include/CommonFunc.h")

add_library(${PROJECT_NAME}_lib STATIC ${CPPSOURCES} ${CPPHEADERS})
add_library(${PROJECT_NAME}_lib::tools ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC "${PROJECT_SOURCE_DIR}/Include")
target_link_libraries(${PROJECT_NAME}_lib)