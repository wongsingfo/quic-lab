//
// Created by Chengke Wong on 2019/9/27.
//

#include "ip_address.h"

#include <algorithm>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>

IPAddress::IPAddress(const in_addr &ipv4_address)
  : address_(),
    family_(IPAddressFamily::IP_V4) {
  address_.v4 = ipv4_address;
}

IPAddress::IPAddress()
  : address_(),
    family_(IPAddressFamily::IP_UNSPEC) {

}

bool operator==(IPAddress lhs, IPAddress rhs) {
  if (lhs.family_ != rhs.family_) {
    return false;
  }

  switch (lhs.family_) {
    case IPAddressFamily::IP_V4:
      return std::equal(lhs.address_.bytes,
                        lhs.address_.bytes + IPAddress::kIPv4AddressSize,
                        rhs.address_.bytes);
    case IPAddressFamily::IP_V6:
      return std::equal(lhs.address_.bytes,
                        lhs.address_.bytes + IPAddress::kIPv6AddressSize,
                        rhs.address_.bytes);
    case IPAddressFamily::IP_UNSPEC:
      return true;
  }

  return false;
}

bool operator!=(IPAddress lhs, IPAddress rhs) {
  return !(lhs == rhs);
}

IPAddress::IPAddress(DataReader *reader)
  : address_(),
    family_(IPAddressFamily::IP_V4) {
  // TODO: support ipv6
  reader->readBytes(&address_.v4.s_addr, IPAddress::kIPv4AddressSize);
}

std::string IPAddress::toString() const {
  char str[INET_ADDRSTRLEN];
  if (family_ != IPAddressFamily::IP_V4 ||
      inet_ntop(AF_INET, &address_.v4, str, INET_ADDRSTRLEN) == nullptr) {
    return "unknown_ipv4_address";
  }
  return str;
}

bool IPAddress::writeTo(DataWriter *writer) {
  if (family_ != IPAddressFamily::IP_V4) {
    LOG(kLogFatal, << "unsupported IP protocol");
    return false;
  }
  return writer->writeBytes(&address_.v4.s_addr, kIPv4AddressSize);
}

bool IPAddress::writeTo(in_addr *dest) {
  if (family_ != IPAddressFamily::IP_V4) {
    LOG(kLogFatal, << "unsupported IP protocol");
    return false;
  }
  *dest = address_.v4;
  return true;
}

IPAddress::IPAddress(const char *text)
  : IPAddress() {
  int rv = inet_pton(AF_INET, text, &address_.v4);
  if (rv < 0) {
    LOG(kLogError, << text << " can't converted into ipv4 address");
    return;
  }

  family_ = IPAddressFamily::IP_V4;
}

IPAddress operator&(IPAddress lhs, IPAddress rhs) {
  if (lhs.family_ != rhs.family_) {
    return IPAddress();
  }
  switch (lhs.family_) {
    case IPAddressFamily::IP_V4:
      return IPAddress(in_addr{
        lhs.address_.v4.s_addr & rhs.address_.v4.s_addr});
    case IPAddressFamily::IP_V6:
      // TODO: support IPv6 mask operation
    case IPAddressFamily::IP_UNSPEC:
      return IPAddress();
  }

  return IPAddress();
}

bool IPAddress::isInaddrAny() const {
  if (family_ != IPAddressFamily::IP_V4) {
    return false;
  }
  return address_.v4.s_addr == INADDR_ANY;
}

IPAddress IPAddress::fromInterfaceName(const char *name) {
  int fd;
  struct ifreq ifr{};
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  /* get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
  if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    close(fd);
    return IPAddress();
  }

  if (close(fd) < 0) {
    return IPAddress();
  }

  return IPAddress(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
}
