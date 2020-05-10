//
// Created by Chengke Wong on 2020/5/9.
//

#include "file_descriptor.h"

#include <unistd.h>

#include "util/easylogging++.h"
#include "util/exception.h"

FileDescriptor::FileDescriptor(int fd) :
        fd_(fd) {}

FileDescriptor::~FileDescriptor() {
    close();
}

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept:
        fd_(other.fd_) {
    other.fd_ = -1;
}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) noexcept {
    if (fd_ >= 0) {
        close();
    }
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
}

void FileDescriptor::close() {
    if (fd_ >= 0) {
        int err = ::close(fd_);
        if (err < 0) {
            LOG(ERROR) << "closing " << fd_;
        }
        fd_ = -1;
    }
}


