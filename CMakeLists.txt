# CMakeList.txt : CMake project for OpCl, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (OpCl "OpCl.cpp")

if(WIN32)
  target_link_libraries(OpSrv wsock32 ws2_32)
endif()

# TODO: Add tests and install targets if needed.
