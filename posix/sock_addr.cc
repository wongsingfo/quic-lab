//
// Created by Chengke Wong on 2020/5/9.
//

#include "sock_addr.h"

#include "util/easylogging++.h"
#include "util/exception.h"

/* error category for getaddrinfo and getnameinfo */
class gai_error_category : public std::error_category {
public:
    const char *name(void) const noexcept override { return "gai_error_category"; }

    std::string message(const int return_value) const noexcept override {
        return gai_strerror(return_value);
    }
};

SockAddr::SockAddr(const SockAddr::Raw &addr, size_t size)
        : SockAddr(addr.as_sockaddr, size) {}

SockAddr::SockAddr(const sockaddr &addr, size_t size)
        : size_(size) {
    DCHECK(size <= sizeof(addr_));
    memcpy(&addr_, &addr, size);
}

SockAddr::SockAddr(const sockaddr_in &addr)
        : size_(sizeof(sockaddr_in)) {
    DCHECK(size_ <= sizeof(addr_));
    memcpy(&addr_, &addr, size_);
}

SockAddr::SockAddr(const std::string &hostname, const std::string &service) {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    *this = SockAddr(hostname, service, hints);
}

SockAddr::SockAddr(const std::string &node, const std::string &service, const addrinfo &hints) {
    /* prepare for the answer */
    addrinfo *resolved_address;

    /* look up the name or names */
    const int gai_ret = getaddrinfo(node.c_str(), service.c_str(), &hints, &resolved_address);
    if (gai_ret) {
        std::string explanation = "getaddrinfo(" + node + ":" + service;
        if (hints.ai_flags | (AI_NUMERICHOST | AI_NUMERICSERV)) {
            explanation += ", numeric";
        }
        explanation += ")";
        throw tagged_error(gai_error_category(), explanation, gai_ret);
    }

    /* if success, should always have at least one entry */
    if (not resolved_address) {
        throw std::runtime_error("getaddrinfo returned successfully but with no results");
    }

    /* put resolved_address in a wrapper so it will get freed if we have to throw an exception */
    std::unique_ptr<addrinfo, std::function<void(addrinfo *)>> wrapped_address
            {resolved_address, [](addrinfo *x) { freeaddrinfo(x); }};

    /* assign to our private members (making sure size fits) */
    *this = SockAddr(*wrapped_address->ai_addr, wrapped_address->ai_addrlen);
}

SockAddr::SockAddr() {}

SockAddr::SockAddr(const std::string &ip, uint16_t port) {
    /* tell getaddrinfo that we don't want to resolve anything */
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    *this = SockAddr(ip, std::to_string(port), hints);
}

bool SockAddr::operator==(const SockAddr &other) const {
    return 0 == memcmp( &addr_, &other.addr_, size_ );
}

