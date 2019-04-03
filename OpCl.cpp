
#include "WSAInitializer.h"
#include "NetworkAddress.h"

#include "SocketClient.h"
#include "OpenThenClose.h"
#include "Protocol.h"

#include <sstream>
#include <iostream>

//void decodeMessage(const std::string& mess)
//{
//}
//
//void scen1(UXSocketMessage& socketMessage)
//{
//  // socketMessage ...
//}

int main()
{
  CWSAInitializer wsaInitializer;

  CNetworkAddress networkAddress { "127.0.0.1", 5001, SOCK_STREAM, IPPROTO_TCP };

  CSocketClient socketClient { networkAddress };

  TOpenThenClose< CSocketClient > socketClientOpened(socketClient);
  if (socketClient.mf_bIsOpen()) {
    CProtocol proto { socketClient.get() };
    std::string mess;
    int idx { -100 };
    EStatus res;
    do {
      proto.sendOk();
      res = proto.recvMessage(mess);
      if (res != EStatus::eOK) {
        fprintf(stderr, "ERROR  (%d)\n", c_eWSALastError);
        break;
      }

      std::cout << "Message: " << mess << std::endl;
    } while( res == EStatus::eOK );
  }

	return 0;
}
