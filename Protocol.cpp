#include "Protocol.h"

#include <cassert>
#include <sstream>

CProtocol::CProtocol( SOCKET s )
    : socket_message { s }
{
}

CProtocol::~CProtocol()
{
  if ( isValid() ) {
   closesocket(socket_message); 
  }
}

bool CProtocol::isValid() const
{
  return ( socket_message != INVALID_SOCKET );
}

EStatus CProtocol::sendOk() const
{
  std::stringstream ss;
  ss << 'K';

  const auto strMess { ss.str() };

  return sendString( strMess );
}

EStatus CProtocol::sendValue(const int val) const
{
  return sendMnemonicWithValue('V', val);
}

EStatus CProtocol::sendOperator(const char op) const
{
  std::stringstream ss;
  ss << "O " << op;

  const auto strMess { ss.str() };

  return sendString( strMess );
}

EStatus CProtocol::sendResult(const int val) const
{
  return sendMnemonicWithValue('R', val);
}

EStatus CProtocol::sendMnemonicWithValue(const char mnem, const int val) const
{
  std::stringstream ss;
  ss << mnem << ' ' << val;

  const auto strMess { ss.str() };

  return sendString( strMess );
}

EStatus CProtocol::sendError(const std::string& str) const
{
  std::stringstream ss;
  ss << "E " << str;

  const auto strMess{ ss.str() };

  return sendString(strMess);
}

EStatus CProtocol::sendString(const std::string& str) const
{
  assert( isValid() );
  if (isValid()) {
    // send message length
    //
    const std::uint32_t strLen { static_cast<std::uint32_t>(str.size()) };
    const auto res1 = rawSend( reinterpret_cast<const char*>(&strLen), sizeof( std::uint32_t) );
    if (res1 == EStatus::eOK) {
      // send message
      //
      const auto res2 = rawSend(str.c_str(), strLen);
      return res2;
    }
    else {
      return res1;
    }
  }
  else {
    return EStatus::eUnknownError;
  }
}

EStatus CProtocol::rawSend(const char* const buff, const int buffLen) const
{
  assert( isValid() );
  if (isValid()) {
    const char* locBuff = buff;
    int locBuffLenSent { 0 };
    int locBuffLenStillToSend = buffLen;
    while (locBuffLenStillToSend > 0) {
      const int sendRes = ::send(socket_message, buff, buffLen, 0); // no flags

      if (sendRes == SOCKET_ERROR) {
        fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
        return EStatus::eConnectionResetByPeer;
      }
      else {
        locBuffLenSent += sendRes;
        locBuffLenStillToSend -= sendRes;
        locBuff += sendRes;

        assert(locBuffLenStillToSend >= 0);
      }
    }
    return EStatus::eOK;
  }
  else {
    fprintf(stderr, "failed to send message; socket is invalid\n");
    return EStatus::eUnknownError;
  }
}

EStatus CProtocol::recvMessage(std::string& str)
{
  assert( isValid() );
  if (isValid()) {
    // recv message length
    //
    std::uint32_t messLen { static_cast< std::uint32_t >( -1 ) }; // intentional
    const auto res1 = rawRecv( reinterpret_cast<char*>( &messLen ), sizeof( std::uint32_t ) );
    if (res1 == EStatus::eOK) {
      // recv message
      //
      str.resize( messLen );
      const auto res2 = rawRecv( str.data() , messLen );
      return res2;
    }
    else {
      return res1;
    }
  }
  else {
    return EStatus::eUnknownError;
  }
}

EStatus CProtocol::rawRecv(char* const buff, const int buffLen)
{
  char* pBuffer = buff;
  int nRecvCount = 0;
  int nStillToRecvCount = buffLen;
  while (nStillToRecvCount > 0 ) {
    const int c_nResult = recv(socket_message, pBuffer, nStillToRecvCount, 0);
      // from the docs: If no error occurs, recv returns the number of bytes received and the buffer pointed to by the
      // buf parameter will contain this data received.
      // If the connection has been gracefully closed, the return value is zero.
    if (c_nResult > 0)
    {
      nRecvCount += c_nResult;
      pBuffer += c_nResult;
      nStillToRecvCount -= c_nResult;
      assert(nStillToRecvCount >= 0);
    }
    else if (c_nResult == 0)
    {
      const int c_eWSALastError = WSAGetLastError();
      assert( c_eWSALastError == 0 );
      printf("INFO connection gracefully closed by remote (apparently)\n");
      closesocket(socket_message);
      return EStatus::eConnectionGracefullyClosed;
    }
    else //(c_nResult < 0)
    {
      const int c_eWSALastError = WSAGetLastError();
      if (c_eWSALastError == WSAETIMEDOUT)
      {
        assert( false ); // not using timeout
        fprintf(stderr, "ERROR recv: timeout\n");
        return EStatus::eTimeOut;
      }
      else if ( c_eWSALastError == WSAECONNRESET )
      {
        fprintf(stderr, "ERROR recv: connection reset by peer\n");
        closesocket(socket_message);
        socket_message = INVALID_SOCKET;
        return EStatus::eConnectionResetByPeer;
      }
      else
      {
        fprintf(stderr, "ERROR recv: undistinguished error (%d)\n", c_eWSALastError);
        closesocket(socket_message);
        socket_message = INVALID_SOCKET;
        return EStatus::eUnknownError;
      }
    }
  }
  return EStatus::eOK;
}

