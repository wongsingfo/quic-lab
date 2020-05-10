//
// Created by Chengke Wong on 2020/5/9.
//

#ifndef QUIC_LAB_FILE_DESCRIPTOR_H
#define QUIC_LAB_FILE_DESCRIPTOR_H

class FileDescriptor {

public:

    explicit FileDescriptor(int fd);

    ~FileDescriptor();

    // disallow copy and assignment
    FileDescriptor(const FileDescriptor &) = delete;

    FileDescriptor &operator=(const FileDescriptor &) = delete;

    FileDescriptor(FileDescriptor &&other) noexcept;

    FileDescriptor &operator=(FileDescriptor &&other) noexcept;

    void close();

    inline int fd_num() const { return fd_; }

private:

    int fd_;

};


#endif //QUIC_LAB_FILE_DESCRIPTOR_H
