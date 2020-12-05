#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "machine.h"
#include "debug.h"
#include "heap.h"
#include "pit.h"
#include "atomic.h"
#include "stdint.h"


class Timer {
	int32_t time;
    ISL lock;
public:
	int TIMELIMIT = 1000; // 1 second
	Timer () : time{0}, lock{} {}

	int32_t get_time() {
		bool disable = lock.lock();
		time = Pit::jiffies;
		lock.unlock(disable);
		return time;
	}
};


class MouseState {

};


class Mouse {
public:
    static Timer* timer;
    static const uint32_t IRQ = 12;
    static const uint32_t VECTOR = 60;
    static const bool Debug = false;
    static void mouse_wait(uint8_t b);
    static void mouse_write(uint8_t write_b);
    static uint8_t mouse_read_command();
    static uint32_t mouse_read_packet();
    static void set_sample_rate(uint32_t rate);
    static uint8_t get_device_id();
    static void set_mouse_mode(uint8_t m);
    static void set_resolution(uint8_t r);
    static void set_scaling(uint8_t s);
    static void init();
};


#endif