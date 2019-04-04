
#include "WSAInitializer.h"
#include "NetworkAddress.h"

#include "SocketClient.h"
#include "OpenThenClose.h"
#include "Protocol.h"
#include "MessageDecoder.h"

#include "anyoption.h"

#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <random>
#include <functional>
#include <thread>
#include <chrono>

bool scen00(CProtocol& prot, int val1, int val2, char op)
{
    EStatus st;

    std::cout << std::endl;

    // send value 1
    //
    st = prot.sendValue(val1);
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
    CMessageDecoder decRespValue1{ mesValue1 };
    if (decRespValue1.getType() != EMessageType::eOK) {
        if (decRespValue1.getType() == EMessageType::eError) {
            std::cout << "ERROR from server: " << decRespValue1.getErrorMessage() << std::endl;
            return true;
        }
        else {
            std::cout << "ERROR from server: <unspecified>" << std::endl;
            return true;
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
    CMessageDecoder decRespValue2{ mesValue2 };
    if (decRespValue2.getType() != EMessageType::eOK) {
        if (decRespValue2.getType() == EMessageType::eError) {
            std::cout << "ERROR from server: " << decRespValue2.getErrorMessage() << std::endl;
            return true;
        }
        else {
            std::cout << "ERROR from server: <unspecified>" << std::endl;
            return true;
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
    CMessageDecoder decRespResult{ mesResult };
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

bool scen01(CProtocol& prot, const int opType, const int value, const char op)
{
    const std::string UNDEF = "deadbeef";
    std::string mesValue;
    std::vector<std::function<EStatus()>> protOperations;

    // send val
    protOperations.push_back([&] {
        std::cout << "sent value: " << value << std::endl;
        return prot.sendValue(value);
    });

    // send op
    protOperations.push_back([&] {
        std::cout << "sent operator: " << op << std::endl;
        return prot.sendOperator(op);
    });

    // receive ACK
    protOperations.push_back([&] {
        return prot.recvMessage(mesValue);
    });

    auto parseResponse = [&] {
        CMessageDecoder decRespResult{ mesValue };
        if (decRespResult.getType() == EMessageType::eResult) {
            const int res = decRespResult.getValue();
            std::cout << "result from server: " << res << std::endl;
        }
        else if (decRespResult.getType() == EMessageType::eError) {
            std::cout << "ERROR from server: " << decRespResult.getErrorMessage() << std::endl;
        }
        else if (decRespResult.getType() != EMessageType::eOK){
            std::cout << "ACK received" << std::endl;
        }
        else {
            std::cout << "ERROR from server: <unspecified>" << std::endl;
        }
        mesValue = UNDEF;
        return true;
    };

    auto checkStatus = [&](EStatus st) {
        if (st != EStatus::eOK) {
            std::cout << "ERROR: COMM: while sending operator: " << statusToMessage(st) << std::endl;
            return false;
        }
        return true;
    };

    auto checkMesg = [&]{
        if (mesValue.compare(UNDEF) != 0) {
            return parseResponse();
        }
        else
        {
            return true;
        }
    };

    if (checkStatus(protOperations[opType]())) {
        return checkMesg();
    }
    else
    {
        return false;
    }
}

char getRandOp(std::uniform_int_distribution<int> dist, std::mt19937& gen)
{
    constexpr std::array< char, 6 > opArray{ '+', '-', '*', '/', '^', '%' };
    const auto index = dist(gen);
    return opArray[index];
}

std::unique_ptr<AnyOption> constructArgumentList()
{
  auto opt = std::make_unique<AnyOption>();

  opt->setFlag('h');
  opt->setOption('s');
  opt->setOption('p');
  opt->setFlag('w');
  //opt->setFlag("erratic");

  opt->addUsage("-s <server name> \n\t\t- server name to connect to (default: localhost");
  opt->addUsage("-p <port number> \n\t\t- port number to connect to (must be above 1024; default: 5001)");
  opt->addUsage("-w \n\t\t- stepped mode; wait after sending each operation (default: false)");
  opt->addUsage("-h \n\t\t- this message and exit");

  return opt;
}

std::tuple<std::string, int, bool> parseOptionsOrGetDefaults(const std::unique_ptr<AnyOption>& opts)
{
  const std::string serv = [&opts]() -> std::string {
    auto s = opts->getValue('s');
    if (s != nullptr) {
      return s;
    }
    else {
      return "127.0.0.1";
    }
  }();

  const int port = [&opts]() -> int {
    auto p = opts->getValue('p');
    if (p != nullptr) {
      int pInt { -1 };
      const auto cnt = sscanf_s(p, "%d", &pInt);
      if (cnt == 1 && pInt >= 1024) {
        return pInt;
      }
      else {
        return 5001;
      }
    }
    else {
      return 5001;
    }
  }();

  const bool stepped = opts->getFlag('w');
  
  return std::make_tuple(serv, port, stepped);
}

int main(int argc, char** argv)
{
    CWSAInitializer wsaInitializer;

  auto opts = constructArgumentList();
  opts->processCommandArgs(argc, argv);
  if (opts->getFlag('h')) {
    opts->printUsage();
    return 0;
  }

  const auto [serverName, portNumber, steppedMode] = parseOptionsOrGetDefaults(opts);

  CNetworkAddress networkAddress { serverName.c_str(), portNumber, SOCK_STREAM, IPPROTO_TCP };

    CSocketClient socketClient{ networkAddress };

    int tryCount{ 100 };
    while (tryCount > 0) {
        TOpenThenClose< CSocketClient > socketClientOpened(socketClient);

        if (socketClient.mf_bIsOpen()) {
            tryCount = 0;

            CProtocol prot{ socketClient.get() };

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> distValue(-50, +50);
            std::uniform_int_distribution<int> distOperator(0, 5);
            std::uniform_int_distribution<int> distOpValue(0, 2);

      bool first{ true };
      bool res { true };
      while (res) {
          int opVal = distOpValue(gen);
          if (first)
          {
              if (opVal == 2)
              {
                  opVal -= 1;
              }
              first = false;
          }
        //res = scen00(prot, distValue(gen), distValue(gen), getRandOp(distOperator, gen));
          res = scen01(prot, opVal, distValue(gen), getRandOp(distOperator, gen));
        if (steppedMode) {
          std::cout << "Press Enter to continue..";
          std::cin.get();
        }
      }
    }
    else {
      --tryCount;
      std::cout << "ERROR connecting to server; waiting for a few seconds... (" << tryCount << " tries left)" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }
  }

    std::cout << "Execution terminated. Press Enter to close..";
    std::cin.get();

    return 0;
}
