#include "string_raw.h"

#include <sstream>
#include <ios>
#include <iomanip>

#include "utility.h"

String String::from_hex(const char* text) {
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

        bool space_hole = false;
        while (*pt && isspace(*pt)) {
            space_hole = true;
            pt += 1;
        }

        if (*pt == '\0') { 
            throw std::runtime_error("the length must be a multiple of 2");
        }

        char c2 = *pt;
        pt += 1;

        if (c1 == '0' && c2 == 'x' && !space_hole) {
            // skip
        } else {
            int byte = std::stoi(std::string(1, c1) + c2, 0, 16);

            DCHECK(0 <= byte && byte <= 255);

            result += (char) (byte);
        }
    }

}

String String::random(size_t size) {
    String result(size);
    for (size_t i = 0; i < size; i++) {
        result[i] = (dtype) i;
    }
    return result;
}

String StringRef::clone() const {
    return String(data(), size());
}

std::string StringRef::to_hex() const {
    std::stringstream stream;
    stream << std::hex;

    for (int i = 0; i < size_; i++) {
        int byte = (int) data_[i];
        stream << std::setw(2) << std::setfill('0') << byte;
    }

    return stream.str();
}

StringRef StringRef::from_text(const char *text) {
    return StringRef((dtype*) text, strlen(text));
}

bool StringRef::operator == (const StringRef &other) const noexcept {
    if (size() != other.size()) {
        return false;
    }

    return 0 == memcmp(data(), other.data(), size());
}

std::ostream &operator<<(std::ostream &os, const StringRef &self) {
    int size = self.size();
    os << "[";
//    os <<  self.size() << " ";

    StringRef::dtype *first = self.data();
    StringRef::dtype *last = first + size;

    os << std::hex;

    for (int i = 0; first != last; first++, i++) {
        if (i > 0 && i % 8 == 0) {
            os << " ";
        }

        int byte = (int) *first;
        os << std::setw(2) << std::setfill('0') << byte;
    }

    os << "]";

    return os;
}

