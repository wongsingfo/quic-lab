//
// Created by Chengke Wong on 2020/5/9.
// Adapted from Maihimaihi
//

#ifndef QUIC_LAB_SOCKET_H
#define QUIC_LAB_SOCKET_H

#include <functional>

#include "util/string_raw.h"
#include "posix/sock_addr.h"
#include "posix/file_descriptor.h"

/* class for network sockets (UDP, TCP, etc.) */
class Socket : public FileDescriptor {
private:
    /* get the local or peer address the socket is connected to */
    SockAddr get_address(const std::string &name_of_function,
                         const std::function<int(int, sockaddr *, socklen_t *)> &function) const;

protected:
    /* default constructor */
    Socket(int domain, int type);

    /* construct from file descriptor */
    Socket(FileDescriptor &&s_fd, int domain, int type);

    /* get and set socket option */
    template<typename option_type>
    socklen_t getsockopt(int level, int option, option_type &option_value) const;

    template<typename option_type>
    void setsockopt(int level, int option, const option_type &option_value);

public:
    /* bind socket to a specified local address (usually to listen/accept) */
    void bind(const SockAddr &address);

    /* connect socket to a specified peer address */
    void connect(const SockAddr &address);

    /* accessors */
    SockAddr local_address() const;

    SockAddr peer_address() const;

    /* allow local address to be reused sooner, at the cost of some robustness */
    void set_reuseaddr();
};

/* UDP socket */
class UDPSocket : public Socket {
public:
    UDPSocket() : Socket(AF_INET, SOCK_DGRAM) {}

    /* receive datagram and where it came from */
    std::tuple<SockAddr, size_t> recvfrom(StringRef data);

    /* send datagram to specified address */
    void sendto(const SockAddr &peer, StringRef payload);

    /* send datagram to connected address */
    void send(StringRef payload);

    /* turn on timestamps on receipt */
    void set_timestamps();
};

/* TCP socket */
class TCPSocket : public Socket {
protected:
    /* constructor used by accept() and SecureSocket() */
    explicit TCPSocket(FileDescriptor &&fd) : Socket(std::move(fd), AF_INET, SOCK_STREAM) {}

public:
    TCPSocket() : Socket(AF_INET, SOCK_STREAM) {}

    /* mark the socket as listening for incoming connections */
    void listen(int backlog);

    /* accept a new incoming connection */
    TCPSocket accept();

    /* original destination of a DNAT connection */
    SockAddr original_dest() const;
};


#endif //QUIC_LAB_SOCKET_H
