#include "NetworkAddress.h"

#include <sstream>
#include <Ws2tcpip.h>

#define gm_Error                                                              \
{                                                                             \
  fprintf (stderr,"Problem in %s: %s\n",__FUNCTION__, gai_strerrorA (lc_r));  \
  exit(1);                                                                    \
}

CNetworkAddress::CNetworkAddress(const char* ac_sHostName, int ac_nPort, int ac_nSockType, int ac_nProtocol, int ac_nFamily)
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

CNetworkAddress::~CNetworkAddress() {
  freeaddrinfo(mc_p);
  mc_p = nullptr;
}

size_t CNetworkAddress::mf_nSize() const {
  return mc_p->ai_addrlen;
}

int CNetworkAddress::mf_nFamily() const {
  return mc_p->ai_family;
}

int CNetworkAddress::mf_nSockType() const {
  return mc_p->ai_socktype;
}

int CNetworkAddress::mf_nProtocol() const {
  return mc_p->ai_protocol;
}

const sockaddr* CNetworkAddress::mf_p() const {
  return mc_p->ai_addr;
}
