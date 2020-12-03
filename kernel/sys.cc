#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "threads.h"
#include "process.h"
#include "machine.h"
#include "ext2.h"
#include "elf.h"
#include "libk.h"
#include "file.h"
#include "heap.h"
#include "shared.h"
#include "kernel.h"
#include "config.h"

using namespace gheith; //use bc took implementations of ext2 and etc...

//Polymorphism from File superclass or whatever... bust virtual and override
class NodeFile : public File {
    Shared<Node> node;
    off_t offset = 0;
    
public:
    NodeFile(Shared<Node> node) : node(node) {}
    
    virtual bool isFile() override {
         return node->is_file(); 
    }
    virtual bool isDirectory() override { 
        return node->is_dir(); 
    }
    virtual off_t size() override { 
        return node->size_in_bytes(); 
    }
    virtual off_t seek(off_t off) override { 
        offset = off; 
        return off; 
    }
    virtual ssize_t read(void* buffer, size_t n) override {
        ssize_t m = node->read(offset, n, (char*) buffer);
        if(m > 0) {
            offset += m;
        }
        return m;
    }
    virtual ssize_t write(void*, size_t) override {
        Debug::panic("bad");
        return -1;
    }
    bool isSymLink() { 
        return node->is_symlink(); 
    }
};


static uint32_t push(uint32_t sp, const char* str) {
    int n = K::strlen(str);
    uint32_t ptr = sp - (n+1);
    ptr = (ptr / 4) * 4;
    memcpy((void*) ptr, str, n + 1);
    return ptr;
}

static uint32_t push(uint32_t sp, uint32_t args) {
    uint32_t ptr = sp - 4;
    *((uint32_t*)ptr) = args;
    return ptr;
}

int SYS::exec(const char* path, int argc, const char* argv[]) { 
    auto file = root_fs->find(root_fs->root, path);
    if(file == nullptr) { return -1; }
    if(!file->is_file()) { return -1; }

    current()->process->clear_private();
    uint32_t sp = 0xefffe000;

    auto ptrs = new uint32_t[argc];
    for(auto i = 0; i < argc; i++) {
        sp = push(sp, argv[i]);
        ptrs[i] = sp;
    }
    sp = push(sp, (uint32_t)0);
    for(auto i = 0; i < argc; i++) {
        sp = push(sp, ptrs[argc-i-1]);
    }
    sp = push(sp,sp);
    sp = push(sp, argc);

    uint32_t entry = ELF::load(file); //
    file == nullptr;
    switchToUser(entry, sp, 0); //
    return -1;
}


extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {

    uint32_t *userEsp = (uint32_t*)frame[3];
    uint32_t userPC = frame[0];
    //Debug::printf("*** syscall#%d, userEsp = %x, userPC = %x\n",eax,userEsp,userPC);
    switch (eax)
    {
        case 0: //exit kill current process
        {
            auto status = userEsp[1];
            current()->process->output->set(status);
            stop(); 
            return 0; // 0 or -1?
        }
        case 1: //write
        {
            int fd = (int) userEsp[1];
            if(fd == 0) {
                return -1;
            }
            char* buf = (char*) userEsp[2];
            uint32_t adr = (uint32_t) buf;
            //check in shared space and APICS
            if((adr < 0x80000000) || adr == kConfig.localAPIC || adr == kConfig.ioAPIC) {
                return -1;
            }
            size_t nbyte = (size_t) userEsp[3];
            auto file = current()->process->getFile(fd);
            if(file == nullptr) {
                 return -1;
            } else {
                return file->write(buf, nbyte);
            }
        }
        case 2: //fork
        {
            auto parent_thr = gheith::current();
            auto parent_pro = parent_thr->process;
            int id;
            auto child_pro = parent_pro->fork(id);
            if(child_pro != nullptr) {
                thread(child_pro, [userPC, userEsp] {
                    switchToUser(userPC, (uint32_t) userEsp, 0);
                });
            }
            return id;
        }
        case 3: //sem
        {
            auto init = userEsp[1];
            return current()->process->newSemaphore(init);
        }
        case 4: //up
        {
            auto id = (int) userEsp[1];
            auto me = current()->process;
            auto sem = me->getSemaphore(id);
            if(sem != nullptr) {
                sem->up();
                return 0;
            }
            else {
                return -1;
            }
        }
        case 5: //down
        {
            auto id = (int) userEsp[1];
            auto me = current()->process;
            auto sem = me->getSemaphore(id);
            if(sem != nullptr) {
                sem->down();
                return 0;
            }
            else {
                return -1;
            }
        }
        case 6: //close
        {
            auto id = (int) userEsp[1];
            auto me = current()->process;
            return me->close(id);
        }
        case 7: //shutdown
        {
            Debug::shutdown();
            //no return
        }
        case 8: //wait
        {
            auto id = (int) userEsp[1];
            auto ptr = (uint32_t*) userEsp[2];
            auto me = current()->process;
            return me->wait(id,ptr);
        }
        case 9: //execl
        {
            auto path = (const char*) userEsp[1];
            auto args = (char**) &userEsp[2];
            int argc = 0;
            while(true) {
                if(args[argc] == 0) break;
                argc++;
            }
            auto copy = new char* [argc+1];
            for(auto i = 0; i < argc; i++) {
                auto len = K::strlen(args[i]);
                copy[i] = (char*) malloc(len+1);
                memcpy(copy[i], args[i], len+1);
            }
            copy[argc] = 0;
            auto status = SYS::exec(path, argc, (const char**) copy);
            return status;
        }
        case 10: //open
        {
            auto path = (const char*) userEsp[1];
            auto node = root_fs->find(root_fs->root, path);
            if(node == nullptr) { 
                return -1; 
            }
            Shared<File> file { new NodeFile(node) };
            auto fd = current()->process->setFile(file);
            return fd;
        }
        case 11: //len
        {
            auto fd = (int) userEsp[1];
            auto file = current()->process->getFile(fd);
            if(file == nullptr) {
                return -1; 
            } else {
                return file->size();
            }
        }
        case 12: //read
        {
            auto fd = (int) userEsp[1];
            char* buf = (char*) userEsp[2];
            size_t nbyte = (size_t) userEsp[3];
            auto file = current()->process->getFile(fd);
            if(file == nullptr) { 
                return -1; 
            }
            return file->read(buf, nbyte);
        }
        case 13: //seek
        {
            auto fd = (int) userEsp[1];
            auto off = (off_t) userEsp[2];
            auto file = current()->process->getFile(fd);
            if(file == nullptr) { 
                return -1; 
            }
            return file->seek(off);
        }
    }
    return -1;
}


void SYS::init(void) { 
    IDT::trap(48,(uint32_t)sysHandler_,3); 
}