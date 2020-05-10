//
// Created by Chengke Wong on 2020/5/9.
//

#ifndef QUIC_LAB_SOCK_ADDR_H
#define QUIC_LAB_SOCK_ADDR_H

#include <netinet/in.h>
#include <netdb.h>
#include <string>

class SockAddr {

public:

    using Raw = union {
        sockaddr as_sockaddr;
        sockaddr_storage as_sockaddr_storage;
    };

    SockAddr();

    SockAddr(const Raw &addr, size_t size);

    SockAddr(const sockaddr &addr, size_t size);

    SockAddr(const sockaddr_in &addr);

    /* construct by resolving host name and service name */
    SockAddr(const std::string &hostname, const std::string &service);

    /* constructor given ip/host, service/port, and optional hints */
    SockAddr(const std::string &node, const std::string &service, const addrinfo &hints);

    /* construct with numerical IP address and numeral port number */
    SockAddr(const std::string &ip, uint16_t port);

    bool operator==(const SockAddr &other) const;

    inline socklen_t size() const { return size_; }

    const sockaddr &to_sockaddr() const { return addr_.as_sockaddr; }

private:

    socklen_t size_;
    Raw addr_;

};


#endif //QUIC_LAB_SOCK_ADDR_H
