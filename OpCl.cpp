
#include "WSAInitializer.h"
#include "NetworkAddress.h"

#include "SocketClient.h"
#include "OpenThenClose.h"
#include "Protocol.h"
#include "MessageDecoder.h"

#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <random>
#include <functional>

bool scen00(CProtocol& prot, int val1, int val2, char op)
{
  EStatus st;

  std::cout << std::endl;

  // send value 1
  //
  st = prot.sendValue( val1 );
  if (st != EStatus::eOK) {
    std::cout << "ERROR: COMM: while sending value: " << statusToMessage(st) << std::endl;
    return false;
  }
  std::cout << "sent value: " << val1 << std::endl;
  
  // recv ACK value 1
  //
  std::string mesValue1;
  st = prot.recvMessage(mesValue1);
  if (st != EStatus::eOK) {
    std::cout << "ERROR: COMM: while receiving acknowledge for value: " << statusToMessage(st) << std::endl;
    return false;
  }
  CMessageDecoder decRespValue1 { mesValue1 };
  if (decRespValue1.getType() != EMessageType::eOK) {
    if (decRespValue1.getType() == EMessageType::eError) {
      std::cout << "ERROR from server: " << decRespValue1.getErrorMessage() << std::endl;
      //return false;
    }
    else {
      std::cout << "ERROR from server: <unspecified>" << std::endl;
      //return false;
    }
  }

  // send value 2
  //
  st = prot.sendValue(val2);
  if (st != EStatus::eOK) {
    std::cout << "ERROR: COMM: while sending value: " << statusToMessage(st) << std::endl;
    return false;
  }
  std::cout << "sent value: " << val2 << std::endl;

  // recv ACK value 2
  //
  std::string mesValue2;
  st = prot.recvMessage(mesValue2);
  if (st != EStatus::eOK) {
    std::cout << "ERROR: COMM: while receiving acknowledge for value: " << statusToMessage(st) << std::endl;
    return false;
  }
  CMessageDecoder decRespValue2 { mesValue2 };
  if (decRespValue2.getType() != EMessageType::eOK) {
    if (decRespValue2.getType() == EMessageType::eError) {
      std::cout << "ERROR from server: " << decRespValue2.getErrorMessage() << std::endl;
      //return false;
    }
    else {
      std::cout << "ERROR from server: <unspecified>" << std::endl;
      //return false;
    }
  }

  // send operator
  //
  st = prot.sendOperator(op);
  if (st != EStatus::eOK) {
    std::cout << "ERROR: COMM: while sending operator: " << statusToMessage(st) << std::endl;
    return false;
  }
  std::cout << "sent operator: " << op << std::endl;

  // recv ACK operator
  //
  std::string mesResult;
  st = prot.recvMessage(mesResult);
  if (st != EStatus::eOK) {
    std::cout << "ERROR: COMM: while receiving result: " << statusToMessage(st) << std::endl;
    return false;
  }
  CMessageDecoder decRespResult { mesResult };
  if (decRespResult.getType() == EMessageType::eResult) {
    const int res = decRespResult.getValue();
    std::cout << "result from server: " << res << std::endl;
    return true;
  }
  else if (decRespResult.getType() == EMessageType::eError) {
    std::cout << "ERROR from server: " << decRespResult.getErrorMessage() << std::endl;
    return true;
  }
  else {
    std::cout << "ERROR from server: <unspecified>" << std::endl;
    return true;
  }
}

char getRandOp(std::uniform_int_distribution<int> dist, std::mt19937& gen)
{
  constexpr std::array< char , 6 > opArray { '+', '-', '*', '/', '^', '%'};
  const auto index = dist(gen);
  return opArray[ index ];
}

int main()
{
  CWSAInitializer wsaInitializer;

  CNetworkAddress networkAddress { "127.0.0.1", 5001, SOCK_STREAM, IPPROTO_TCP };

  CSocketClient socketClient { networkAddress };

  {
    TOpenThenClose< CSocketClient > socketClientOpened(socketClient);
    if (socketClient.mf_bIsOpen()) {
      CProtocol prot { socketClient.get() };

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> distValue(-50, +50);
      std::uniform_int_distribution<int> distOperator(0, 5);

      bool res { true };
      while (res) {
        res = scen00(prot, distValue(gen), distValue(gen), getRandOp(distOperator, gen));
      }
    }
    else {
      // TODO
    }
  }

  std::cout << "Execution terminated. Press any key..";
  getchar();

	return 0;
}
