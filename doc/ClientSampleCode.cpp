#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <type_traits>

#include <codecvt>
#include <vector>
#include <Ws2tcpip.h>
#include <cassert>

      CWSAInitializer::CWSAInitializer()
          : mv_bInitialized(false)
      {
        WSADATA d;
        const auto c_WSAStartupResult = ::WSAStartup((2 << 8) + 2, &d);
        assert(c_WSAStartupResult == 0);
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

#pragma comment(lib, "IPHLPAPI.lib")

#define gm_Error                                                              \
{                                                                             \
  fprintf (stderr,"Problem in %s: %s\n",__FUNCTION__, gai_strerrorA (lc_r));  \
  exit(1);                                                                    \
}

      template <>
      void CNetworkAddressSourceSupplier::mp_SetAsMulticastInterface<AF_INET>(int ac_nSocket) {
        assert(mc_p->ai_family == AF_INET);

        const auto lv_p = reinterpret_cast<const sockaddr_in*>(mf_p());
        in_addr lc_Addr = lv_p->sin_addr;
        int lc_r = setsockopt(ac_nSocket, IPPROTO_IP, IP_MULTICAST_IF, (char*)&lc_Addr, sizeof(lc_Addr));
        if (lc_r != 0) gm_Error;
      }

      template <>
      void CNetworkAddressSourceSupplier::mp_SetAsMulticastInterface<AF_INET6>(int ac_nSocket) {
        assert(mc_p->ai_family == AF_INET6);

#if 0
        const auto lv_pAddress = reinterpret_cast<const sockaddr_in6*>(mf_p());
        in6_addr lc_Addr = lv_pAddress->sin6_addr;

        unsigned long lv_nSize = 10000;
        IP_ADAPTER_ADDRESSES* lv_AdapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(lv_nSize);

        {
          ULONG lc_r = GetAdaptersAddresses(AF_INET6, 0, nullptr, lv_AdapterAddresses, &lv_nSize);
          if (lc_r != 0) gm_Error;
        }

        int lv_nFoundIndex = -1;

        PIP_ADAPTER_MULTICAST_ADDRESS lv_p = lv_AdapterAddresses->FirstMulticastAddress;
        int lv_nIndex = 0;
        while (lv_p != nullptr) {
          if (memcmp(&lv_p->Address.lpSockaddr->sa_data[0], &lc_Addr, sizeof(lc_Addr)) == 0) {
            lv_nFoundIndex = lv_nIndex;
            break;
          }
          lv_nIndex += 1;
          lv_p = lv_p->Next;
        }

        if (lv_nFoundIndex == -1) {
          fprintf(stderr, "Found no multicast interface with this address\n");
          exit(1);
        }
#else
        int lv_nFoundIndex = 0;
#endif
        int lc_r = setsockopt(ac_nSocket, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char*)&lv_nFoundIndex, sizeof(lv_nFoundIndex));
        if (lc_r != 0) gm_Error;
      }

      CNetworkAddressSourceSupplier::CNetworkAddressSourceSupplier(const char* ac_sHostName, int ac_nPort, int ac_nSockType, int ac_nProtocol, int ac_nFamily)
        : mc_p(nullptr)
      {
        struct addrinfo lv_Hints = { 0 };

        lv_Hints.ai_family = ac_nFamily;
        lv_Hints.ai_socktype = ac_nSockType;
        lv_Hints.ai_protocol = ac_nProtocol;

        std::stringstream ss;
        ss << ac_nPort;

        unsigned int lc_r = getaddrinfo(ac_sHostName, ss.str().c_str(), &lv_Hints, &mc_p);
        if (lc_r != 0) gm_Error;
        if (mc_p->ai_next != nullptr) {
          fprintf(stderr, "Ambiguous in %s: more than 1 answer\n", __FUNCTION__);
          exit(1);
        }
      }

      CNetworkAddressSourceSupplier::~CNetworkAddressSourceSupplier() {
        freeaddrinfo(mc_p);
        mc_p = nullptr;
      }

      size_t CNetworkAddressSourceSupplier::mf_nSize() const {
        return mc_p->ai_addrlen;
      }

      int CNetworkAddressSourceSupplier::mf_nFamily() const {
        return mc_p->ai_family;
      }

      int CNetworkAddressSourceSupplier::mf_nSockType() const {
        return mc_p->ai_socktype;
      }

      int CNetworkAddressSourceSupplier::mf_nProtocol() const {
        return mc_p->ai_protocol;
      }

      const sockaddr* CNetworkAddressSourceSupplier::mf_p() const {
        return mc_p->ai_addr;
      }

      void CNetworkAddressSourceSupplier::mp_SetAsMulticastInterface(int ac_nSocket) {
        switch (mc_p->ai_family) {
        case AF_INET: mp_SetAsMulticastInterface<AF_INET>(ac_nSocket); break;
        case AF_INET6: mp_SetAsMulticastInterface<AF_INET6>(ac_nSocket); break;
        default: assert(false);
        }
      }


      CStreamingSourceSupplier::CStreamingSourceSupplier( const std::string& ac_sInterfaceAddress, const int ac_nPort )
        : // wsa init will be the first thing that happens
          mc_nSocket( INVALID_SOCKET )
        , mv_NetworkAddress(ac_sInterfaceAddress.c_str() , ac_nPort, SOCK_STREAM, IPPROTO_TCP)
      {
      }

      CStreamingSourceSupplier::~CStreamingSourceSupplier()
      {
        mp_Close();
      }

      void CStreamingSourceSupplier::mp_Open()
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

          // timeout on socket
          DWORD timeOutMilli = static_cast< DWORD >( sc_dfSocketReceiveTimeoutSeconds * 1000 );
          if ( setsockopt( mc_nSocket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeOutMilli, sizeof( timeOutMilli ) ) < 0 ) {
            const int c_eWSALastError = WSAGetLastError();
            fprintf(stderr, "ERROR: setting timeout on socket: %i\n", c_eWSALastError);
            perror("ERROR setting timeout on socket");
            closesocket(mc_nSocket);
            mc_nSocket = INVALID_SOCKET;
            mc_WSAInit.mp_Cleanup();
          }
          else {
            printf("SUCCESS setting timeout on socket\n");
          }
        }
      }

      void CStreamingSourceSupplier::mp_Close()
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

      bool CStreamingSourceSupplier::mf_bIsOpen() const
      {
        return ( mc_nSocket != INVALID_SOCKET );
      }

      CStreamingSourceSupplier::mt_eRecvStatus CStreamingSourceSupplier::mf_eRecv(::MBSTProtocol::MBSTMessage& av_Message)
      {
        assert( mc_nSocket != INVALID_SOCKET );

        // message length
        std::int32_t nLengthBuffer;
          // TODO: use the same std::int32_t type on the server side to ensure consistency
        const auto c_RawRecvMessageLengthResult = mf_nRawRecv(reinterpret_cast<char*>(&nLengthBuffer), sizeof(std::int32_t));
        const auto c_eRawRecvMessageLengthStatus = std::get< 1 >( c_RawRecvMessageLengthResult );
        switch ( c_eRawRecvMessageLengthStatus ) {
          case mc_eOK: {
            const auto c_nRawRecvMessageLength_Value = std::get< 0 >( c_RawRecvMessageLengthResult );
            if ( c_nRawRecvMessageLength_Value == sizeof( std::int32_t ) ) {

              if ( nLengthBuffer > 0 ) {
                // message
                std::vector< char > bufferVec( nLengthBuffer );
                const auto c_RawRecvMessageDataResult = mf_nRawRecv(bufferVec.data(), nLengthBuffer);
                const auto c_eRawRecvMessageDataStatus = std::get< 1 >( c_RawRecvMessageDataResult );
                switch ( c_eRawRecvMessageDataStatus ) {
                  case mc_eOK: {
                    const auto c_nRawRecvMessageData_MessageLength = std::get< 0 >( c_RawRecvMessageDataResult );
                    if ( c_nRawRecvMessageData_MessageLength == nLengthBuffer ) {
                      const bool c_bParseResult = av_Message.ParseFromArray( bufferVec.data() , nLengthBuffer );
                      if ( c_bParseResult ) {
                        return mc_eOK;
                      } else {
                        return mc_eScrambledData;
                      }
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
              assert( c_nRawRecvMessageLength_Value == sizeof( std::int32_t ) );
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

      CStreamingSourceSupplier::mt_RecvInfo CStreamingSourceSupplier::mf_nRawRecv(char* const ac_pBuffer, const int ac_nCount)
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
            return std::make_tuple( nRecvCount  , mc_eConnectionGracefullyClosed );
          }
          else //(c_nResult < 0)
          {
            const int c_eWSALastError = WSAGetLastError();
            if (c_eWSALastError == WSAETIMEDOUT)
            {
              fprintf(stderr, "ERROR recv: timeout\n");
              return std::make_tuple( nRecvCount , mc_eTimeOut );
            }
            else if ( c_eWSALastError == WSAECONNRESET )
            {
              fprintf(stderr, "ERROR recv: connection reset by peer\n");
              closesocket(mc_nSocket);
              mc_nSocket = INVALID_SOCKET;
              return std::make_tuple( nRecvCount , mc_eConnectionResetByPeer );
            }
            else
            {
              fprintf(stderr, "ERROR recv: undistinguished error (%d)\n", c_eWSALastError);
              closesocket(mc_nSocket);
              mc_nSocket = INVALID_SOCKET;
              return std::make_tuple( nRecvCount , mc_eUnknownError );
            }
          }
        }
        return std::make_tuple( nRecvCount , mc_eOK );
      }

/*
          , mc_StreamingSource(
                sf_sReadStringFromIniAsStdString( _T("ExternalSimulation.ini") , _T("TargetMachine") , _T("IPv4Addr") , "127.0.0.1" )
              , sf_nReadIntFromIni              ( _T("ExternalSimulation.ini") , _T("TargetMachine") , _T("IPv4Port") , 9901 ) )
*/

      namespace {
        template < typename at_ >
        struct TOpenClose {
          TOpenClose( at_& ac_)
              : mv_( ac_)
          {
            assert( false == mv_.mf_bIsOpen() );
            mv_.mp_Open();
          }
          ~TOpenClose()
          {
            mv_.mp_Close();
            assert( false == mv_.mf_bIsOpen() );
          }
          const at_* operator->() const
          {
            return &mv_;
          }

        private:
          at_& mv_;
        };
      }

      void CExternalSimulationSourceSupplierThread::mp_Execute()
      {
        bool bSuccessfullyConnected = false;
        {
          TOpenClose< CStreamingSourceSupplier > connOpenClose( mc_StreamingSource );

          bSuccessfullyConnected = connOpenClose->mf_bIsOpen();
          if ( bSuccessfullyConnected ) {
            mp_ExecuteReceiveAndDispatchLoop();
          }
        } // connection will be closed here

        if ( !bSuccessfullyConnected ) {
          // TODO: for the moment this is all we can do
          // at the very least, this is supposed to give an indication in the UI that something is wrong down here
          // maybe we could extend this with a new message type which could help a measurement controller do more

          // then notify strategy that something bad has happened
        }
      }

      void CExternalSimulationSourceSupplierThread::mp_ExecuteReceiveAndDispatchLoop()
      {
        using DataModelI::Base::CAdcTime;
        using DataModelI::Base::CAdcTimeInterval;

        ::MBSTProtocol::MBSTMessage lv_Message;
        bool bEofReceived = false;
        bool bIrrecoverableError = false;
        bool bDataExhausted = false; // TODO: this batch of flags calls for an enum

        while (!bEofReceived && !bIrrecoverableError && !bDataExhausted)
        {
          // data received in frames for each and every channel
          switch ( mc_StreamingSource.mf_eRecv( lv_Message ) ) {
            case CStreamingSourceSupplier::mc_eOK : {
              break;
            }
            case CStreamingSourceSupplier::mc_eTimeOut : {
              // do nothing, wait for the next message
              // TODO: we could monitor consecutive timeout errors and then be able to
              // send some AT's deriving some information out of this
              break;
            }
            case CStreamingSourceSupplier::mc_eConnectionGracefullyClosed:
            case CStreamingSourceSupplier::mc_eConnectionResetByPeer:
            case CStreamingSourceSupplier::mc_eScrambledData: {
              bIrrecoverableError = true;
              break;
            }
            case CStreamingSourceSupplier::mc_eDataExhausted: {
              bDataExhausted = true;
              break;
            }
            case CStreamingSourceSupplier::mc_eUnknownError : {
              assert( "UnknownError needs to be properly translated" == nullptr );
              break;
            }
            default:
              assert( "MessageReceiveStatus not properly handled" == nullptr );
          }
        }
      }

namespace UnitTesting {
  // NOTE: following unit tests are present here because, one time or another,
  // they were useful in testing different parts of the network acquisition logic;
  // they are left here because they might still be useful in the future

  //void sp_GenericEndlessTCPStreamedSocketClient()
  //{
  //  static const int PROCDEC_DEFAULT_BUFLEN = 2000000;
  //  static const char PROCDEC_DEFAULT_ADDRESS[] = "146.122.109.159";
  //  static const char PROCDEC_DEFAULT_PORT[] = "9901";

  //  WSADATA wsaData;
  //  SOCKET ConnectSocket = INVALID_SOCKET;
  //  struct addrinfo *result = NULL,
  //                  *ptr = NULL,
  //                  hints;
  //  char *sendbuf = "this is a test";
  //  char recvbuf[PROCDEC_DEFAULT_BUFLEN];
  //  int iResult;
  //  int recvbuflen = PROCDEC_DEFAULT_BUFLEN;

  //  //// Validate the parameters
  //  //if (argc != 2) {
  //  //    printf("usage: %s server-name\n", argv[0]);
  //  //    return 1;
  //  //}

  //  // Initialize Winsock
  //  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  //  if (iResult != 0) {
  //      printf("WSAStartup failed with error: %d\n", iResult);
  //      return;
  //  }

  //  ZeroMemory( &hints, sizeof(hints) );
  //  hints.ai_family = AF_UNSPEC;
  //  hints.ai_socktype = SOCK_STREAM;
  //  hints.ai_protocol = IPPROTO_TCP;

  //  // Resolve the server address and port
  //  iResult = getaddrinfo(PROCDEC_DEFAULT_ADDRESS, PROCDEC_DEFAULT_PORT, &hints, &result);
  //  if ( iResult != 0 ) {
  //      printf("getaddrinfo failed with error: %d\n", iResult);
  //      WSACleanup();
  //      return;
  //  }

  //  // Attempt to connect to an address until one succeeds
  //  for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
  //      // Create a SOCKET for connecting to server
  //      ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
  //          ptr->ai_protocol);
  //      if (ConnectSocket == INVALID_SOCKET) {
  //          printf("socket failed with error: %ld\n", WSAGetLastError());
  //          WSACleanup();
  //          return;
  //      }

  //      // Connect to server.
  //      iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
  //      if (iResult == SOCKET_ERROR) {
  //          closesocket(ConnectSocket);
  //          ConnectSocket = INVALID_SOCKET;
  //          continue;
  //      }
  //      break;
  //  }

  //  freeaddrinfo(result);

  //  if (ConnectSocket == INVALID_SOCKET) {
  //      printf("Unable to connect to server!\n");
  //      WSACleanup();
  //      return;
  //  }

  //  //// Send an initial buffer
  //  //iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
  //  //if (iResult == SOCKET_ERROR) {
  //  //    printf("send failed with error: %d\n", WSAGetLastError());
  //  //    closesocket(ConnectSocket);
  //  //    WSACleanup();
  //  //    return 1;
  //  //}
  //  //printf("Bytes Sent: %ld\n", iResult);

  //  // shutdown the connection since no more data will be sent
  //  iResult = shutdown(ConnectSocket, SD_SEND);
  //  if (iResult == SOCKET_ERROR) {
  //      printf("shutdown failed with error: %d\n", WSAGetLastError());
  //      closesocket(ConnectSocket);
  //      WSACleanup();
  //      return;
  //  }

  //  // Receive until the peer closes the connection
  //  do {
  //      iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
  //      if ( iResult > 0 )
  //          printf("Bytes received: %d\n", iResult);
  //      else if ( iResult == 0 )
  //          printf("Connection closed\n");
  //      else
  //          printf("recv failed with error: %d\n", WSAGetLastError());
  //  } while( iResult > 0 );

  //  // cleanup
  //  closesocket(ConnectSocket);
  //  WSACleanup();
  //}
  //gm_FWI_UT_UnitTest(sp_GenericEndlessTCPStreamedSocketClient, "sp_GenericEndlessTCPStreamedSocketClient", "");


  //void sp_MBSTConnection_TestAndConsumeDataUsingProtocolVersion2()
  //{
  //  ::MBSTProtocol::MBSTMessage lv_Message;
  //  CStreamingSourceSupplier mc_StreamingSource(
  //            sf_sReadStringFromIniAsStdString( _T("ExternalSimulation.ini") , _T("TargetMachine") , _T("IPv4Addr") , "127.0.0.1" )
  //          , sf_nReadIntFromIni              ( _T("ExternalSimulation.ini") , _T("TargetMachine") , _T("IPv4Port") , 9901 ) );
  //  TOpenClose< CStreamingSourceSupplier > connOpenClose( mc_StreamingSource );

  //  // bookkeeping
  //  const unsigned int mc_nSourceBlockStreamCount = 2;
  //  unsigned long long mv_nFirstProcessedAbsoluteSampleIndex { std::numeric_limits< unsigned long long >::max() };
  //  unsigned long long mv_nExpectedToReceiveAndProcessRelativeSampleIndex { std::numeric_limits< unsigned long long >::max() };

  //  bool bEofReceived = false;
  //  bool bIrrecoverableError = false;
  //  bool bDataExhausted = false;
  //  while ( !bEofReceived && !bIrrecoverableError && !bDataExhausted) {
  //    switch ( mc_StreamingSource.mf_eRecv( lv_Message ) ) {
  //      case CStreamingSourceSupplier::mc_eOK : {
  //        if ( lv_Message.is_eof() ) {
  //          bEofReceived = true;
  //          break;
  //        }

  //        //std::cout << lv_Message.model_name() << " ________________\n";

  //        // init
  //        const int c_nCurrentMessageValidSampleIndex = 0;
  //        if ( mv_nFirstProcessedAbsoluteSampleIndex == std::numeric_limits< unsigned long long >::max() ) {
  //          mv_nFirstProcessedAbsoluteSampleIndex = lv_Message.frame_index();
  //          mv_nExpectedToReceiveAndProcessRelativeSampleIndex = 0;
  //        } else {
  //          //gm_FWI_UT_LogExp(mv_nExpectedToReceiveAndProcessRelativeSampleIndex == (lv_Message.frame_index() - mv_nFirstProcessedAbsoluteSampleIndex));
  //        }

  //        // checking
  //        const int c_nCurrentMessageReceivedXSampleCount = lv_Message.frames_size();
  //        const int c_nCurrentMessageVariableCount = lv_Message.frames(0).simulation_variable_size();
  //        //gm_FWI_UT_LogExp( c_nCurrentMessageVariableCount == mc_nSourceBlockStreamCount );
  //        const int c_nCurrentMessageToBeConsumedSampleCount = c_nCurrentMessageReceivedXSampleCount - c_nCurrentMessageValidSampleIndex;
  //        //gm_FWI_UT_LogExp( c_nCurrentMessageToBeConsumedSampleCount > 0 );

  //        // logging
  //        //std::cout << "\t frame_index = "              << lv_Message.frame_index()
  //        //          << "\t frames_size = "              << c_nCurrentMessageReceivedXSampleCount
  //        //          << "\t simulation_variable_size = " << c_nCurrentMessageVariableCount << '\n';

  //        // usage X
  //        for ( const auto& c_Frame : lv_Message.frames() ) {
  //          std::cout
  //              <<                      std::setw(12) << c_Frame.tv_sec()
  //              << '.'
  //              << std::setfill('0') << std::setw( 9) << c_Frame.tv_nsec()
  //              << " , "
  //              << std::setprecision(15) << c_Frame.simulation_variable( 0 )
  //              << '\n';
  //        }

  //        // usage Y
  //        ::MBSTProtocol::MBSTFrame const* const* const c_ppFrame = lv_Message.frames().data();
  //        int nChannelIndex = 0;
  //        for ( auto nIdx = 0; nIdx < c_nCurrentMessageVariableCount ; nIdx++ , nChannelIndex++ ) {
  //          VXVector< double > yBlockBuffer( c_nCurrentMessageReceivedXSampleCount , vxNoInit );
  //          int nDestIndex = 0;
  //          int nSourceIndex = c_nCurrentMessageValidSampleIndex;
  //          for ( ; nSourceIndex < c_nCurrentMessageReceivedXSampleCount ; nDestIndex++, nSourceIndex++ ) {
  //            ::MBSTProtocol::MBSTFrame const& c_Frame = *c_ppFrame[ nSourceIndex ];
  //            const double c_dfValue = c_Frame.simulation_variable( nChannelIndex );
  //            yBlockBuffer[ nDestIndex ] = c_dfValue;
  //          }
  //          //std::cout << "    channel " << nChannelIndex << " with " << yBlockBuffer.length() << " frames\n";
  //        }

  //                                    //const auto c_nXLen = timeXAxisVec.length();
  //                                    //const auto c_nYLen = dataYAxisVec.length();
  //                                    //assert( c_nXLen == c_nYLen );
  //                                    //for (int nDbgIdx = 0 ; nDbgIdx < c_nXLen ; nDbgIdx++ ) {
  //                                    //  std::cout
  //                                    //      << std::setprecision(15) << timeXAxisVec( nDbgIdx )
  //                                    //      << ","
  //                                    //      << std::setprecision(15) << dataYAxisVec( nDbgIdx )
  //                                    //      << '\n';
  //                                    //}

  //        // bookkeeping update
  //        mv_nExpectedToReceiveAndProcessRelativeSampleIndex
  //            = lv_Message.frame_index() + c_nCurrentMessageReceivedXSampleCount
  //                - mv_nFirstProcessedAbsoluteSampleIndex;

  //        break;
  //      }
  //      case CStreamingSourceSupplier::mc_eTimeOut : {
  //        // do nothing, wait for the next message
  //        // TODO: we could monitor consecutive timeout errors and then be able to
  //        // send some AT's deriving some information out of this
  //        std::cout << "TIMEOUT\n";
  //        break;
  //      }
  //      case CStreamingSourceSupplier::mc_eConnectionGracefullyClosed:
  //      case CStreamingSourceSupplier::mc_eConnectionResetByPeer:
  //      case CStreamingSourceSupplier::mc_eScrambledData: {
  //        bIrrecoverableError = true;
  //        break;
  //      }
  //      case CStreamingSourceSupplier::mc_eDataExhausted: {
  //        bDataExhausted = true;
  //        break;
  //      }
  //      case CStreamingSourceSupplier::mc_eUnknownError : {
  //        gm_FWI_UT_LogExp( "UnknownError needs to be properly translated" == nullptr );
  //        break;
  //      }
  //      default:
  //        gm_FWI_UT_LogExp( "MessageReceiveStatus not properly handled" == nullptr );
  //    }
  //  }

  //  if ( bIrrecoverableError ) {
  //    gm_FWI_UT_LogExp( "Irrecoverable error" == nullptr );
  //  } else {
  //    if ( bEofReceived ) {
  //      gm_FWI_UT_LogExp( "Eof received" );
  //    } else {
  //      gm_FWI_UT_LogExp( "Unspecified error" == nullptr );
  //    }
  //  }
  //}
  //gm_FWI_UT_UnitTest(sp_MBSTConnection_TestAndConsumeDataUsingProtocolVersion2, "sp_MBSTConnection_TestAndConsumeDataUsingProtocolVersion2", "");

  void sp_TestStreamingSourceSupplier()
  {
    // just making sure that initialization does not call 'exit'
    CStreamingSourceSupplier sssUnlikelyToBeOpen( std::string("127.0.0.1") , 65530 );
    sssUnlikelyToBeOpen.mp_Open();
    gm_FWI_UT_LogExp( sssUnlikelyToBeOpen.mf_bIsOpen() == false );

    CStreamingSourceSupplier sssServerMessageBlock( std::string("127.0.0.1") , 445 );
    sssServerMessageBlock.mp_Open();
    gm_FWI_UT_LogExp( sssServerMessageBlock.mf_bIsOpen() == true );
  }
  gm_FWI_UT_UnitTest(sp_TestStreamingSourceSupplier, "sp_TestStreamingSourceSupplier", "");
}
