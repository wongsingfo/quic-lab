#include "util/string.h"

#include <sstream>
#include <ios>
#include <iomanip>

String String::from_text(const char* text) {
    std::string result;

    const char* pt = text;
    while (true) {
        while (*pt && isspace(*pt)) {
            pt += 1;
        }
        if (*pt == '\0') {
            return String((unsigned char*) result.data(), result.size());
        }

        char c1 = *pt;
        pt += 1;
        while (*pt && isspace(*pt)) {
            pt += 1;
        }

        if (*pt == '\0') { 
            throw std::runtime_error("the length must be a multiple of 2");
        }

        char c2 = *pt;
        pt += 1;

        int byte = std::stoi(std::string(1, c1) + c2, 0, 16);

        result += (char) (byte);
    }

}

std::string String::to_string() const {
    std::stringstream stream;
    stream << std::hex;

    for (int i = 0; i < size_; i++) {
        int byte = (int) data_[i];
        stream << std::setw(2) << std::setfill('0') << byte;
    }

    return stream.str();
}

