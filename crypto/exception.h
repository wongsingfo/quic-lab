#include <openssl/err.h>

namespace crypto {

class openssl_error : public std::runtime_error {

private:
    int code_;

public:
    openssl_error(const std::string &s_attempt, int _)
        : runtime_error(s_attempt) {

        /* OpenSSL maintans a error queue for each thread, so we need
         * to get the error code from the queue */

        int code = ERR_get_error();
        if (code == 0) {
            throw std::runtime_error("unknown error in OpenSSL");
        }

        code_ = code;
    }

    const char* what() const noexcept override {
        /* ERR_error_string() returns a pointer to a static 
         * buffer containing the string if buf == NULL*/

        char *msg = ERR_error_string(code_, /* buf = */ NULL);
        return msg;
    }

};

inline int openssl_call(const std::string &s_attempt, int return_value) {
    if (return_value > 0) {
        return return_value;
    }
    throw openssl_error(s_attempt, return_value);
}

} // namespace crypto
