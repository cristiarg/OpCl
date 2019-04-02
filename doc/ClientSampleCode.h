#include <Winsock2.h>

//#pragma warning (push, 3)
#include <LmsHq/FunctionalC/ExternalSimulation/model_frame.pb.h>
//#pragma warning (pop)

    class CWSAInitializer final {
    public:
      CWSAInitializer();
      ~CWSAInitializer();
      void mp_Cleanup();
    private:
      bool mv_bInitialized;
    };

    class CNetworkAddressSourceSupplier final {
    public:
      CNetworkAddressSourceSupplier(const char* ac_sHostName, int ac_nPort, int ac_nSockType, int ac_nProtocol, int ac_nFamily = AF_UNSPEC);
      ~CNetworkAddressSourceSupplier();

      size_t mf_nSize() const;
      int mf_nFamily() const;
      int mf_nSockType() const;
      int mf_nProtocol() const;
      const sockaddr* mf_p() const;

      void mp_SetAsMulticastInterface(int ac_nSocket);
    private:
      addrinfo* mc_p;

      template <unsigned int ac_nFamily>
      void mp_SetAsMulticastInterface(int ac_nSocket);
    };

    class CStreamingSourceSupplier final {
    public:
      enum mt_eRecvStatus {
            mc_eOK
          , mc_eTimeOut
          , mc_eConnectionGracefullyClosed
          , mc_eConnectionResetByPeer
              // the difference between the two above is kind of fuzzy
          , mc_eScrambledData
          , mc_eDataExhausted
              // there are two ways of signalling end of data by the server
              //  - the regular one: using the application protocol
              //  - the second one is this, and it is exceptional as it usually
              //    means that something is wrong on the server side
          , mc_eUnknownError
      };

    private:
      typedef std::tuple< int , mt_eRecvStatus > mt_RecvInfo;

    public:
      CStreamingSourceSupplier( const std::string& ac_sInterfaceAddress, const int ac_nPort );
      ~CStreamingSourceSupplier();

      mt_eRecvStatus mf_eRecv(::MBSTProtocol::MBSTMessage& av_Message);

      void mp_Open();
      void mp_Close();
      bool mf_bIsOpen() const;

    private:
      mt_RecvInfo mf_nRawRecv(char* const ac_pBuffer, const int ac_nCount);

    private:
      CWSAInitializer mc_WSAInit;
        // NOTE: must be the first member to ensure wsa subsystem initialization is the first thing that happens
      SOCKET mc_nSocket;
      CNetworkAddressSourceSupplier mv_NetworkAddress;
    };

    class CExternalSimulationSourceSupplierThread : public [subsystem(LmsHq,FrameworkI,Thread)] CFunction {
      public:
        void mp_Execute() override;

      private:
        void mp_ExecuteReceiveAndDispatchLoop();

      private:
        CStreamingSourceSupplier mc_StreamingSource;
    };
