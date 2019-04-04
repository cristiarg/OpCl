# CMakeList.txt : CMake project for OpCl, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (OpCl
    "WSAInitializer.cpp" "WSAInitializer.h"
    "NetworkAddress.cpp" "NetworkAddress.h"
    "SocketClient.cpp" "SocketClient.h"
    "OpenThenClose.h"
    "Protocol.cpp" "Protocol.h"
    "MessageDecoder.cpp" "MessageDecoder.h"
    "anyoption.cpp" "anyoption.h"
    "OpCl.cpp")

add_definitions(/std:c++17)

if(WIN32)
  target_link_libraries(OpCl wsock32 ws2_32)
endif()

# TODO: Add tests and install targets if needed.
