
#include "WSAInitializer.h"
#include "NetworkAddress.h"

#include "SocketMessage.h"
#include "OpenThenClose.h"

#include <iostream>

int main()
{
  CWSAInitializer wsaInitializer;

  CNetworkAddress networkAddress { "127.0.0.1", 5001, SOCK_STREAM, IPPROTO_TCP };
  CSocketMessage socketMessage { networkAddress };
  //CSocketMessage socketMessage { /*"localhost"*/ "127.0.0.1" , 5001 };

  TOpenThenClose< CSocketMessage > socketMessageOpened(socketMessage);

  std::string mess;
  do {
    socketMessage.mf_eRecv(mess);
    std::cout << "Message: " << mess << std::endl;
  } while( mess.size() > 0 );

	return 0;
}
