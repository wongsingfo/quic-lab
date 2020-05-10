//
// Created by Chengke Wong on 2020/5/9.
//

#include "socket.h"
/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>

#include "util/utility.h"
#include "util/exception.h"

/* default constructor for socket of (subclassed) domain and type */
Socket::Socket(const int domain, const int type)
        : FileDescriptor(system_call("socket", socket(domain, type, 0))) {}

/* construct from file descriptor */
Socket::Socket(FileDescriptor &&fd, const int domain, const int type)
        : FileDescriptor(std::move(fd)) {
    int actual_value;
    socklen_t len;

#ifdef SO_DOMAIN
    /* verify domain */
    len = getsockopt( SOL_SOCKET, SO_DOMAIN, actual_value );
    if ( (len != sizeof( actual_value )) or (actual_value != domain) ) {
        throw std::runtime_error( "socket domain mismatch" );
    }
#endif

    /* verify type */
    len = getsockopt(SOL_SOCKET, SO_TYPE, actual_value);
    if ((len != sizeof(actual_value)) or (actual_value != type)) {
        throw std::runtime_error("socket type mismatch");
    }
}

/* get the local or peer address the socket is connected to */
SockAddr Socket::get_address(const std::string &name_of_function,
                             const std::function<int(int, sockaddr *, socklen_t *)> &function) const {
    SockAddr::Raw address;
    socklen_t size = sizeof(address);

    system_call(name_of_function, function(fd_num(),
                                           &address.as_sockaddr,
                                           &size));

    return SockAddr(address, size);
}

SockAddr Socket::local_address() const {
    return get_address("getsockname", getsockname);
}

SockAddr Socket::peer_address() const {
    return get_address("getpeername", getpeername);
}

/* bind socket to a specified local address (usually to listen/accept) */
void Socket::bind(const SockAddr &address) {
    system_call("bind", ::bind(fd_num(),
                              &address.to_sockaddr(),
                              address.size()));
}

/* connect socket to a specified peer address */
void Socket::connect(const SockAddr &address) {
    system_call("connect", ::connect(fd_num(),
                                    &address.to_sockaddr(),
                                    address.size()));
}

/* send datagram to specified address */
void UDPSocket::sendto(const SockAddr &peer, StringRef payload) {
    static constexpr ssize_t RECEIVE_MTU = 65536;
    DCHECK(payload.size() <= RECEIVE_MTU);

    const ssize_t bytes_sent =
            system_call("sendto", ::sendto(fd_num(),
                                           payload.data(),
                                           payload.size(),
                                           0,
                                           &peer.to_sockaddr(),
                                           peer.size()));

    if (size_t(bytes_sent) != payload.size()) {
        throw std::runtime_error("datagram payload too big for sendto()");
    }
}

/* send datagram to connected address */
void UDPSocket::send(StringRef payload) {
    const ssize_t bytes_sent =
            system_call("send", ::send(fd_num(),
                                      payload.data(),
                                      payload.size(),
                                      0));

    if (size_t(bytes_sent) != payload.size()) {
        throw std::runtime_error("datagram payload too big for send()");
    }
}

/* mark the socket as listening for incoming connections */
void TCPSocket::listen(const int backlog) {
    system_call("listen", ::listen(fd_num(), backlog));
}

/* accept a new incoming connection */
TCPSocket TCPSocket::accept(void) {
    return TCPSocket(FileDescriptor(
            system_call("accept",
                        ::accept(fd_num(), nullptr, nullptr))));
}

/* get socket option */
template<typename option_type>
socklen_t Socket::getsockopt(const int level, const int option, option_type &option_value) const {
    socklen_t optlen = sizeof(option_value);
    system_call("getsockopt", ::getsockopt(fd_num(), level, option,
                                          &option_value, &optlen));
    return optlen;
}

/* set socket option */
template<typename option_type>
void Socket::setsockopt(const int level, const int option, const option_type &option_value) {
    system_call("setsockopt", ::setsockopt(fd_num(), level, option,
                                          &option_value, sizeof(option_value)));
}

/* allow local address to be reused sooner, at the cost of some robustness */
void Socket::set_reuseaddr(void) {
    setsockopt(SOL_SOCKET, SO_REUSEADDR, int(true));
}

/* turn on timestamps on receipt */
void UDPSocket::set_timestamps(void) {
#ifdef SO_TIMESTAMPNS
    setsockopt(SOL_SOCKET, SO_TIMESTAMPNS, int(true));
#elif SO_TIMESTAMP
    setsockopt(SOL_SOCKET, SO_TIMESTAMP, int(true));
#endif
}

std::tuple<SockAddr, size_t> UDPSocket::recvfrom(StringRef data) {
    static const ssize_t RECEIVE_MTU = 65536;

    /* receive source address and payload */
    SockAddr::Raw datagram_source_address;

    socklen_t fromlen = sizeof(datagram_source_address);

    ssize_t recv_len = system_call("recvfrom",
                                   ::recvfrom(fd_num(),
                                              data.data(),
                                              data.size(),
                                              MSG_TRUNC,
                                              &datagram_source_address.as_sockaddr,
                                              &fromlen));

    return std::make_tuple(SockAddr(datagram_source_address, fromlen),
                           recv_len);
}


SockAddr TCPSocket::original_dest() const {
#ifdef SO_ORIGINAL_DST
    SockAddr::Raw dstaddr;
    socklen_t len = getsockopt(SOL_IP, SO_ORIGINAL_DST, dstaddr);
    return SockAddr(dstaddr, len);
#else
    throw std::runtime_error("unimplemented!");
#endif
}

