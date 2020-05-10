//
// Created by Chengke Wong on 2020/5/10.
// Adapted from Maihimaihi
//

#ifndef QUIC_LAB_SECURE_SOCKET_H
#define QUIC_LAB_SECURE_SOCKET_H

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "posix/socket.h"
#include "util/string.h"

class SecureSocket;

class SSLContext {
public:

    explicit SSLContext(bool is_client);

    SecureSocket new_secure_socket(TCPSocket &&sock);

private:
    struct CTX_deleter {
        void operator()(SSL_CTX *x) const { SSL_CTX_free(x); }
    };

    typedef std::unique_ptr<SSL_CTX, CTX_deleter> CTX_handle;
    CTX_handle ctx_;
};

class SecureSocket : public TCPSocket {
    friend class SSLContext;

public:

    void connect();

    void accept();

    size_t read(StringRef data);

    void write(StringRef message);

private:
    struct SSL_deleter {
        void operator()(SSL *x) const { SSL_free(x); }
    };

    typedef std::unique_ptr<SSL, SSL_deleter> SSL_handle;
    SSL_handle ssl_;

    SecureSocket(TCPSocket &&sock, SSL *ssl);
};

#endif //QUIC_LAB_SECURE_SOCKET_H
