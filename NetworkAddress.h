#pragma once

#include <Winsock2.h>

class CNetworkAddress final {
public:
  CNetworkAddress(const char* ac_sHostName, int ac_nPort, int ac_nSockType, int ac_nProtocol, int ac_nFamily = AF_UNSPEC);
  ~CNetworkAddress();

  size_t mf_nSize() const;
  int mf_nFamily() const;
  int mf_nSockType() const;
  int mf_nProtocol() const;
  const sockaddr* mf_p() const;

private:
  addrinfo* mc_p;
};

