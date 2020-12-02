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

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    using namespace gheith;

    uint32_t *userEsp = (uint32_t*)frame[3];
    uint32_t userPC = frame[0];

    Debug::printf("*** syscall#%d, userEsp = %x, userPC = %x\n",eax,userEsp,userPC);

    auto currentProcess = current()->process;
    
    switch(eax) {
        case 0: // exit
            {
                int rc = (int)userEsp[1];
                currentProcess->exit(rc);
            }
            break;
        case 7: // shutdown
            Debug::shutdown();
            break;
        default:
            Debug::panic("Not a proper system call\n");

    }

    return -1;

}

/*
    switch(eax) {
        case 0: // exit
            {
                int rc = (int)userEsp[1];
                currentProcess->exit(rc);
            }
            break;
        case 1: // write
            {
                int fd = (int)userEsp[1];
                void* buf = (void*)userEsp[2];
                size_t nbyte = (size_t)userEsp[3];
                auto file = currentProcess->getFile(fd);
                if (file != nullptr) {
                    return file->write(buf, nbyte);
                } else {
                    return -1;
                }
            }
            break;
        case 2: // fork
            {
                int id = 1;
                auto child_process = currentProcess->fork(id);

                thread(child_process, [userPC, userEsp] {
                    switchToUser(userPC, (uint32_t)userEsp, 0);
                });

                return id;
            }
            break;
        case 3: // sem
            {
                uint32_t initial = (uint32_t)userEsp[1];
                return currentProcess->newSemaphore(initial);
            }
            break;
        case 4: // up
            {
                int id = (int)userEsp[1];
                auto sem = currentProcess->getSemaphore(id);
                if (sem != nullptr) {
                    sem->up();
                    return 0;
                } else {
                    return -1;
                }
            }
            break;
        case 5: // down
            {
                int id = (int)userEsp[1];
                auto sem = currentProcess->getSemaphore(id);
                if (sem != nullptr) {
                    sem->down();
                    return 0;
                } else {
                    return -1;
                }
            }
            break;
        case 6: // close
            {
                int id = (int)userEsp[1];
                return currentProcess->close(id);
            }
            break;
        case 7: // shutdown
            Debug::shutdown();
            break;
        case 8: // wait
            {
                int id = (int)userEsp[1];
                uint32_t *status = (uint32_t*)userEsp[2];
                return currentProcess->wait(id, status);
            }
            break;
        case 9: // execl
            {
                const char* path = (const char*)userEsp[1];
                int argc = 0;
                while (userEsp[2 + argc]) {
                    argc++;
                }
                char** argv = new char*[argc];
                for (int i = 0; i < argc; i++) {
                    argv[i] = (char*)userEsp[2 + i];
                }
                SYS::exec(path, argc, (const char**)argv);
            }
            break;
        case 10: // open
            {
                const char* fn = (const char*)userEsp[1];
                auto root = root_fs->root;
                auto fileNode = root_fs->find(root, fn); // Shared<Node>
                auto openFile = new OpenFile(fileNode);
                if (openFile->isDirectory()) {
                    Debug::panic("The path refers to a directory\n");
                }
                Shared<File> openFileShared{ openFile };
                return currentProcess->setFile(openFileShared);
            }
            break;
        case 11: // len
            {
                int fd = (int)userEsp[1];
                auto file = currentProcess->getFile(fd);
                if (file != nullptr) {
                    return file->size();
                } else {
                    return -1;
                }
            }
            break;
        case 12: // read
            {
                int fd = (int)userEsp[1];
                void* buf = (void*)userEsp[2];
                size_t nbyte = (size_t)userEsp[3];
                auto file = currentProcess->getFile(fd);
                if (file != nullptr) {
                    return file->read(buf, nbyte);
                } else {
                    return -1;
                }
            }
            break;
        case 13: // seek
            {
                int fd = (int)userEsp[1];
                off_t offset = (off_t)userEsp[2];
                auto file = currentProcess->getFile(fd);
                return file->seek(offset);
            }
            break;
        default:
            Debug::panic("Not a proper system call\n");

    }
*/

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}

int SYS::exec(const char* path, int argc, const char* argv[]) {
    using namespace gheith;
    
    auto root = root_fs->root;
    auto file = root_fs->find(root, path);

    // number of chars in argv in total including '\0'
    uint32_t num_char = 0;
    
    for (int i = 0; i < argc; i++) {
        num_char += ((uint32_t) K::strlen(argv[i])) + 1;
    }

    // number of 4B blocks needed for argv
    int num_blocks = (num_char / 4) + ((num_char % 4 == 0) ? 0 : 1);

    // space to store bytes, argc, and argv
    // numbers represent: argc, **argv, *argv[0]-*argv[argc-1], num_blocks, argv characters
    // addresses represents memories in kernel, later need to modify values
    auto arg_copy = malloc( (1 + 1 + argc + 1 + num_blocks) * sizeof(uint32_t) );

    // argc
    *((uint32_t*)arg_copy + 0) = argc;

    // **argv
    *((uint32_t*)arg_copy + 1) = (uint32_t)arg_copy + 8;

    // num_blocks
    *((uint32_t*)arg_copy + 1 + 1 + argc) = num_blocks;

    // address of argv entries start
    uint32_t argv_entry = (uint32_t)arg_copy + (1 + 1) * sizeof(uint32_t);

    // address of argv characters start and will iterate over the whole characters
    uint32_t argv_char = (uint32_t)arg_copy + (1 + 1 + argc + 1) * sizeof(uint32_t);

    for (int i = 0; i < argc; i++) {
        *((uint32_t*)argv_entry + i) = argv_char;
        const char* tmp = argv[i];
        while (*tmp != '\0') {
            *((char*)argv_char) = *tmp;
            argv_char++;
            tmp++;
        }
        *((char*)argv_char) = '\0';
        argv_char++;
    }

    current()->process->clear_private();

    uint32_t userEIP = ELF::load(file);  // userEIP
    uint32_t userESP = 0xefffe000; // userESP

    // offset, address of the heap
    uint32_t offset = (uint32_t)arg_copy;

    // argc from heap
    uint32_t saved_argc = *((uint32_t*)arg_copy + 0);

    // num_blocks from heap
    uint32_t saved_num_blocks = *((uint32_t*)arg_copy + 1 + 1 + saved_argc);

    // size of data stored in user stack from heap
    uint32_t data_size = (1 + 1 + argc + 1 + num_blocks) * sizeof(uint32_t);

    // decrease esp to store data
    userESP -= data_size;

    // store argc in the user stack
    *((uint32_t*)userESP + 0) = saved_argc;

    // store **argv in the user stack
    *((uint32_t*)userESP + 1) = *((uint32_t*)arg_copy + 1) - offset + userESP;

    // store *argv[0]-*argv[argc-1]
    for (uint32_t i = 0; i < saved_argc; i++) {
        *((uint32_t*)userESP + 2 + i) = *((uint32_t*)arg_copy + 2 + i) - offset + userESP;
    }

    // store NULL to indicate the end of argv
    *((uint32_t*)userESP + 2 + saved_argc) = 0;

    memcpy( (void*)((uint32_t*)userESP + 3 + saved_argc), (void*)((uint32_t*)arg_copy + 3 + saved_argc), saved_num_blocks * sizeof(uint32_t) );

    free(arg_copy);

    switchToUser(userEIP, userESP, 0); // after iret, the next code starts from userEIP, which is start in crt0.S

    return -1;
}
