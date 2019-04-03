#pragma once

#include "NetworkAddress.h"

#include <Winsock2.h>

#include <string>
#include <tuple>

class CSocketMessage final {
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
  typedef std::tuple< mt_eRecvStatus , int > mt_RecvInfo;

public:
  CSocketMessage( /*const std::string& ac_sInterfaceAddress, const int ac_nPort*/ const CNetworkAddress& na );
  ~CSocketMessage();

  mt_eRecvStatus mf_eRecv(std::string& str);

  void mp_Open();
  void mp_Close();
  bool mf_bIsOpen() const;

private:
  mt_RecvInfo mf_nRawRecv(char* const ac_pBuffer, const int ac_nCount);

private:
  const CNetworkAddress& mv_NetworkAddress;
  SOCKET mc_nSocket;
};
