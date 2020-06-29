//
// Created by Chengke Wong on 2019/9/27.
//

#ifndef SRC_IP_IP_ADDRESS_H
#define SRC_IP_IP_ADDRESS_H


#include <netinet/in.h>
#include "base/data_writer.h"
#include "base/data_reader.h"

enum class IPAddressFamily {
  IP_V4,
  IP_V6,
  IP_UNSPEC,
};

class IPAddress {
 public:
  static const size_t kIPv4AddressSize = 32 / 8;
  static const size_t kIPv6AddressSize = 128 / 8;
  static const size_t kMaxIpAddressSize = kIPv6AddressSize;

  IPAddress();

  IPAddress(const IPAddress &other) = default;

  IPAddress(const char *text);

  explicit IPAddress(DataReader *reader);

  explicit IPAddress(const in_addr &ipv4_address);

  static IPAddress fromInterfaceName(const char *name);

  IPAddress &operator=(const IPAddress &other) = default;

  IPAddress &operator=(IPAddress &&other) = default;

  friend bool operator==(IPAddress lhs, IPAddress rhs);

  friend bool operator!=(IPAddress lhs, IPAddress rhs);

  friend IPAddress operator&(IPAddress lhs, IPAddress rhs);

  static IPAddress fullMask() {
    return {"255.255.255.255"};
  }

  static IPAddress inaddrAny() {
    return {"0.0.0.0"};
  }

  bool writeTo(DataWriter *writer);

  bool writeTo(in_addr *dest);

  std::string toString() const;

  struct IpAddressHash {
    inline size_t operator()(const IPAddress &address) const {
      return address.address_.v4.s_addr;
    }
  };

  bool isInaddrAny() const;

  inline bool isSpecified() const {
    return family_ != IPAddressFamily::IP_UNSPEC;
  }

 private:
  union {
    in_addr v4;
    in6_addr v6;
    char bytes[kMaxIpAddressSize];
  } address_;
  IPAddressFamily family_;
};


#endif //SRC_IP_IP_ADDRESS_H
