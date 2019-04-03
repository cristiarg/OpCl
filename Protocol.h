#pragma once

#include <memory>
#include <string>
#include <Ws2tcpip.h>

enum class EStatus {
      eOK
    , eTimeOut
    , eConnectionGracefullyClosed
    , eConnectionResetByPeer
        // the difference between the two above is kind of fuzzy
    //, eScrambledData
    //, eDataExhausted
        // there are two ways of signalling end of data by the server
        //  - the regular one: using the application protocol
        //  - the second one is this, and it is exceptional as it usually
        //    means that something is wrong on the server side
    , eUnknownError
};

static std::string statusToMessage(const EStatus st)
{
  switch (st) {
    case EStatus::eOK                         : return "OK";
    case EStatus::eTimeOut                    : return "Timeout";
    case EStatus::eConnectionGracefullyClosed : return "Connection gracefully closed";
    case EStatus::eConnectionResetByPeer      : return "Connection reset by peer";
    case EStatus::eUnknownError               : return "Unknown error";
    default:
      return std::string();
      break;
  }
}

class CProtocol final {
public:
  CProtocol( SOCKET s );
  ~CProtocol();

  bool isValid() const;

  EStatus sendOk() const;
  EStatus sendValue(const int val) const;
  EStatus sendOperator(const char op) const;
  EStatus sendResult (const int val) const;
  EStatus sendError (const std::string& str) const;

  EStatus recvMessage(std::string& str);

  EStatus sendString(const std::string& str) const;
private:
  EStatus sendMnemonicWithValue(const char mnem, const int val) const;
  EStatus rawSend(const char* const buff, const int buffLen) const;

  EStatus rawRecv(char* const buff, const int buffMaxLen);

private:
  SOCKET socket_message;
};

using UXProtcol = std::unique_ptr< CProtocol >;
