#ifndef _barrier_h_
#define _barrier_h_

#include "atomic.h"
#include "semaphore.h"
#include "shared.h"

class Barrier {
    Semaphore sem;
    Atomic<uint32_t> togo;
    Atomic<uint32_t> ref_count;
public:
    Barrier(uint32_t count) : sem(0), togo(count), ref_count(0) {}

    Barrier(const Barrier&) = delete;
    
    void sync() {
        if (togo.add_fetch(-1) == 0) {
            sem.up();
        } else {
            sem.down();
            sem.up();
        }
    }

    friend class Shared<Barrier>;
};

#endif

