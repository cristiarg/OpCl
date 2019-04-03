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
    , eScrambledData
    , eDataExhausted
        // there are two ways of signalling end of data by the server
        //  - the regular one: using the application protocol
        //  - the second one is this, and it is exceptional as it usually
        //    means that something is wrong on the server side
    , eUnknownError
};

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
