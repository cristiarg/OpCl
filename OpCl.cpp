
#include "SocketMessage.h"
#include "OpenThenClose.h"

#include <iostream>

int main()
{
  CSocketMessage socketMessage { /*"localhost"*/ "127.0.0.1" , 5001 };
  TOpenThenClose< CSocketMessage > socketMessageOpened(socketMessage);

  std::string mess;
  do {
    socketMessage.mf_eRecv(mess);
    std::cout << "Message: " << mess << std::endl;
  } while( mess.size() > 0 );

	return 0;
}
