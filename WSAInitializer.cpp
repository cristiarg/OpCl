#include "WSAInitializer.h"

#include <cassert>
#include <Ws2tcpip.h>

CWSAInitializer::CWSAInitializer()
    : mv_bInitialized(false)
{
  WSADATA d;
  const auto c_WSAStartupResult = ::WSAStartup((2 << 8) + 2, &d);
  assert(c_WSAStartupResult == 0);
  if (c_WSAStartupResult != 0) {
    ::WSACleanup();
  }
  mv_bInitialized = true;
}

void CWSAInitializer::mp_Cleanup()
{
  if (mv_bInitialized) {
    ::WSACleanup();
    mv_bInitialized = false;
  }
}

CWSAInitializer::~CWSAInitializer()
{
  if (mv_bInitialized) {
    ::WSACleanup();
  }
}
