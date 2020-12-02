#ifndef _U8250_H_
#define _U8250_H_

/* 8250 */

#include "io.h"
#include "file.h"
#include "shared.h"
#include "ext2.h"

class U8250 : public OutputStream<char> {
public:
    U8250() { }
    virtual void put(char ch);
    virtual char get();
};

class U8250File : public File {
    U8250 *it;
public:
    U8250File(U8250* it) : it(it) {}
    bool isFile() override { return true; }
    bool isDirectory() override { return false; }
    off_t seek(off_t offset) { return offset; }
    off_t size() { return 0x7FFFFFFF; }
    ssize_t read(void* buffer, size_t n) {
        if (n == 0) return 0;
        *((char*)buffer) = it->get();
        return 1;
    }
    ssize_t write(void* buffer, size_t n) {
        if (n == 0) return 0;
        it->put(*((char*)buffer));
        return 1;
    }
};

class OpenFile : public U8250File {
    Shared<Node> node;
    off_t fileOffset;
    off_t fileSize;
public:
    OpenFile(Shared<Node> fileNode) : U8250File(nullptr), node(fileNode), fileOffset(0), fileSize((off_t) node->size_in_bytes()) {}

    bool isFile() override { 
        return node->is_file() || node->is_symlink();
    }

    bool isDirectory() override { 
        return node->is_dir();
    }

    off_t size() override {
        return fileSize;
    }

    off_t seek(off_t offset) override { 
        fileOffset = offset;
        return fileOffset;
    }

    ssize_t read(void* buffer, size_t n) override {
        auto bytes = node->read_all((uint32_t) fileOffset, (uint32_t) n, (char*) buffer);
        if (bytes < 0) {
            Debug::panic("Offset out of bound\n");
        }
        fileOffset += (off_t) bytes;
        return (ssize_t) bytes;
    }

    ssize_t write(void* buffer, size_t n) override {
        auto bytes = node->write_all((uint32_t) fileOffset, (uint32_t) n, (char*) buffer);
        if (bytes < 0) {
            Debug::panic("Offset out of bound\n");
        }
        fileOffset += (off_t) bytes;
        return (ssize_t) bytes;
    }
};

#endif
