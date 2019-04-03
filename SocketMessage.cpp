#include "SocketMessage.h"

#include <cassert>
#include <vector>

CSocketMessage::CSocketMessage( const std::string& ac_sInterfaceAddress, const int ac_nPort )
  : // wsa init will be the first thing that happens
    mc_nSocket( INVALID_SOCKET )
  , mv_NetworkAddress(ac_sInterfaceAddress.c_str() , ac_nPort, SOCK_STREAM, IPPROTO_TCP)
{
}

CSocketMessage::~CSocketMessage()
{
  mp_Close();
}

void CSocketMessage::mp_Open()
{
  if ( mc_nSocket == INVALID_SOCKET ) {
    // only one address should have been found; connect to it

    // create socket
    mc_nSocket = socket(mv_NetworkAddress.mf_nFamily(), mv_NetworkAddress.mf_nSockType(), mv_NetworkAddress.mf_nProtocol());
    if (mc_nSocket < 0) {
      fprintf(stderr, "ERROR: opening stream socket\n");
      perror("ERROR opening stream socket");
      mc_WSAInit.mp_Cleanup();
      mc_nSocket = INVALID_SOCKET;
    }
    else {
      printf("SUCCESS creating stream socket\n");
    }

    // connect socket
    const auto c_nResultConnect = connect(mc_nSocket, mv_NetworkAddress.mf_p(), mv_NetworkAddress.mf_nSize());
    if (c_nResultConnect == SOCKET_ERROR) {
      fprintf(stderr, "ERROR: connecting stream socket\n");
      perror("ERROR connecting stream socket");
      closesocket(mc_nSocket);
      mc_nSocket = INVALID_SOCKET;
      mc_WSAInit.mp_Cleanup();
    }
    else {
      printf("SUCCESS connecting stream socket\n");
    }

    //// timeout on socket
    //DWORD timeOutMilli = static_cast< DWORD >( /*sc_dfSocketReceiveTimeoutSeconds*/ 5 * 1000 );
    //if ( setsockopt( mc_nSocket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeOutMilli, sizeof( timeOutMilli ) ) < 0 ) {
    //  const int c_eWSALastError = WSAGetLastError();
    //  fprintf(stderr, "ERROR: setting timeout on socket: %i\n", c_eWSALastError);
    //  perror("ERROR setting timeout on socket");
    //  closesocket(mc_nSocket);
    //  mc_nSocket = INVALID_SOCKET;
    //  mc_WSAInit.mp_Cleanup();
    //}
    //else {
    //  printf("SUCCESS setting timeout on socket\n");
    //}
  }
}

void CSocketMessage::mp_Close()
{
  if ( mc_nSocket != INVALID_SOCKET ) {
    // first, shutdown
    int n = ::shutdown(mc_nSocket, SD_BOTH);
    if ( n != 0 ) {
      const int c_nWSALastError = ::WSAGetLastError();
      switch ( c_nWSALastError ) {
        case WSAECONNABORTED :
          fprintf(stderr, "ERROR: shutting down socket: WSAECONNABORTED\n");
          perror("ERROR: shutting down socket: WSAECONNABORTED");
          break;
        case WSAECONNRESET :
          fprintf(stderr, "ERROR: shutting down socket: WSAECONNRESET\n");
          perror("ERROR: shutting down socket: WSAECONNRESET");
          break;
        case WSAEINPROGRESS :
          fprintf(stderr, "ERROR: shutting down socket: WSAEINPROGRESS\n");
          perror("ERROR: shutting down socket: WSAEINPROGRESS");
          break;
        case WSAEINVAL :
          fprintf(stderr, "ERROR: shutting down socket: WSAEINVAL\n");
          perror("ERROR: shutting down socket: WSAEINVAL");
          break;
        case WSAENETDOWN :
          fprintf(stderr, "ERROR: shutting down socket: WSAENETDOWN\n");
          perror("ERROR: shutting down socket: WSAENETDOWN");
          break;
        case WSAENOTCONN :
          fprintf(stderr, "ERROR: shutting down socket: WSAENOTCONN\n");
          perror("ERROR: shutting down socket: WSAENOTCONN");
          break;
        case WSAENOTSOCK :
          fprintf(stderr, "ERROR: shutting down socket: WSAENOTSOCK\n");
          perror("ERROR: shutting down socket: WSAENOTSOCK");
          break;
        case WSANOTINITIALISED :
          fprintf(stderr, "ERROR: shutting down socket: WSANOTINITIALISED\n");
          perror("ERROR: shutting down socket: WSANOTINITIALISED");
          break;
        default :
          assert( false );
      }
    }
    // then, deallocate
    ::closesocket(mc_nSocket);
    mc_nSocket = INVALID_SOCKET;
  }
}

bool CSocketMessage::mf_bIsOpen() const
{
  return ( mc_nSocket != INVALID_SOCKET );
}

CSocketMessage::mt_eRecvStatus CSocketMessage::mf_eRecv(std::string& str)
{
  assert( mc_nSocket != INVALID_SOCKET );

  // message length
  //
  std::int32_t nLengthBuffer;
  const auto c_RawRecvMessageLengthResult = mf_nRawRecv(reinterpret_cast<char*>(&nLengthBuffer), sizeof(std::int32_t));
  const auto c_eRawRecvMessageLengthStatus = std::get< 0 >( c_RawRecvMessageLengthResult );
  switch ( c_eRawRecvMessageLengthStatus ) {
    case mc_eOK: {
      const auto c_nRawRecvMessageLengthValue = std::get< 1 >( c_RawRecvMessageLengthResult );
      if ( c_nRawRecvMessageLengthValue == sizeof( std::int32_t ) ) {

        if ( nLengthBuffer > 0 ) {
          // message
          //
          str.resize( nLengthBuffer );
          const auto c_RawRecvMessageDataResult = mf_nRawRecv(str.data(), nLengthBuffer);
          const auto c_eRawRecvMessageDataStatus = std::get< 0 >( c_RawRecvMessageDataResult );
          switch ( c_eRawRecvMessageDataStatus ) {
            case mc_eOK: {
              const auto c_nRawRecvMessageData_MessageLength = std::get< 1 >( c_RawRecvMessageDataResult );
              if ( c_nRawRecvMessageData_MessageLength == nLengthBuffer ) {
                return mc_eOK;
              } else {
                assert( c_nRawRecvMessageData_MessageLength == nLengthBuffer );
                return mc_eScrambledData;
              }
              break;
            }
            case mc_eTimeOut:
            case mc_eConnectionGracefullyClosed:
            case mc_eConnectionResetByPeer:
            {
              return c_eRawRecvMessageDataStatus;
              break;
            }
            case mc_eUnknownError: {
              assert( "UnknownError needs to be properly translated" == nullptr );
              return mc_eUnknownError;
              break;
            }
            default:
              assert( "MessageLengthStatus not properly handled" == nullptr );
          }
        } else {
          // we receive a zero size, not at the end of the simulation, but
          // when data is exhausted due to exceptional scenarios (e.g.: crashes)
          return mc_eDataExhausted;
        }

      } else {
        // scrambled message
        assert( c_nRawRecvMessageLengthValue == sizeof( std::int32_t ) );
        return mc_eScrambledData;
      }
      break;
    }
    case mc_eTimeOut:
    case mc_eConnectionGracefullyClosed:
    case mc_eConnectionResetByPeer:
    {
      return c_eRawRecvMessageLengthStatus;
      break;
    }
    case mc_eUnknownError: {
      assert( "UnknownError needs to be properly translated" == nullptr );
      return mc_eUnknownError;
      break;
    }
    default:
      assert( "MessageLengthStatus not properly handled" == nullptr );
  }

  return mc_eOK;
}

CSocketMessage::mt_RecvInfo CSocketMessage::mf_nRawRecv(char* const ac_pBuffer, const int ac_nCount)
{
  char* pBuffer = ac_pBuffer;
  int nRecvCount = 0;
  int nStillToRecvCount = ac_nCount;
  while (nStillToRecvCount > 0 ) {
    const int c_nResult = recv(mc_nSocket, pBuffer, nStillToRecvCount, 0);
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
      return std::make_tuple( mc_eConnectionGracefullyClosed , nRecvCount  );
    }
    else //(c_nResult < 0)
    {
      const int c_eWSALastError = WSAGetLastError();
      if (c_eWSALastError == WSAETIMEDOUT)
      {
        fprintf(stderr, "ERROR recv: timeout\n");
        return std::make_tuple( mc_eTimeOut , nRecvCount );
      }
      else if ( c_eWSALastError == WSAECONNRESET )
      {
        fprintf(stderr, "ERROR recv: connection reset by peer\n");
        closesocket(mc_nSocket);
        mc_nSocket = INVALID_SOCKET;
        return std::make_tuple( mc_eConnectionResetByPeer , nRecvCount );
      }
      else
      {
        fprintf(stderr, "ERROR recv: undistinguished error (%d)\n", c_eWSALastError);
        closesocket(mc_nSocket);
        mc_nSocket = INVALID_SOCKET;
        return std::make_tuple( mc_eUnknownError , nRecvCount );
      }
    }
  }
  return std::make_tuple( mc_eOK , nRecvCount );
}

//CSocketMessage::mt_RecvInfo CSocketMessage::mf_nRawRecv(char* const ac_pBuffer, const int ac_nCount)
//{
//  char* pBuffer = ac_pBuffer;
//  int nRecvCount = 0;
//  int nStillToRecvCount = ac_nCount;
//  while (nStillToRecvCount > 0 ) {
//    const int c_nResult = recv(mc_nSocket, pBuffer, nStillToRecvCount, 0);
//      // from the docs: If no error occurs, recv returns the number of bytes received and the buffer pointed to by the
//      // buf parameter will contain this data received.
//      // If the connection has been gracefully closed, the return value is zero.
//    if (c_nResult > 0)
//    {
//      nRecvCount += c_nResult;
//      pBuffer += c_nResult;
//      nStillToRecvCount -= c_nResult;
//      assert(nStillToRecvCount >= 0);
//    }
//    else if (c_nResult == 0)
//    {
//      const int c_eWSALastError = WSAGetLastError();
//      assert( c_eWSALastError == 0 );
//      printf("INFO connection gracefully closed by remote (apparently)\n");
//      return std::make_tuple( nRecvCount  , mc_eConnectionGracefullyClosed );
//    }
//    else //(c_nResult < 0)
//    {
//      const int c_eWSALastError = WSAGetLastError();
//      if (c_eWSALastError == WSAETIMEDOUT)
//      {
//        fprintf(stderr, "ERROR recv: timeout\n");
//        return std::make_tuple( nRecvCount , mc_eTimeOut );
//      }
//      else if ( c_eWSALastError == WSAECONNRESET )
//      {
//        fprintf(stderr, "ERROR recv: connection reset by peer\n");
//        closesocket(mc_nSocket);
//        mc_nSocket = INVALID_SOCKET;
//        return std::make_tuple( nRecvCount , mc_eConnectionResetByPeer );
//      }
//      else
//      {
//        fprintf(stderr, "ERROR recv: undistinguished error (%d)\n", c_eWSALastError);
//        closesocket(mc_nSocket);
//        mc_nSocket = INVALID_SOCKET;
//        return std::make_tuple( nRecvCount , mc_eUnknownError );
//      }
//    }
//  }
//  return std::make_tuple( nRecvCount , mc_eOK );
//}
