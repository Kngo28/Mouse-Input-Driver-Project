#ifndef _PS2INTERFACE_H_
#define _PS2INTERFACE_H_

#include "machine.h"
#include "debug.h"
#include "heap.h"

// need interrupt and atomic protections

// need a Timer class, or try to use jiffies (pit.h/cc)
// sample class
class Timer {
	int32_t time;
public:
	int TIMELIMIT = 2; // seconds
	Timer () : time{0} {}

	void start_timer() {
		// start time
	}

	int32_t end_timer() {
		// end time
		// returns last recorded time
	}

	int32_t get_time() {
		return time;
	}

	// check time against limit?
};

class PS2INTERFACE {

	Timer timer;

public:
    
	PS2INTERFACE::PS2INTERFACE () : timer{} {}

	// before_read and before_write should probably do the polling process
	// maybe even keep a timer?

	bool before_read() {
        // read bit 0 of Status Register byte
        int status = inb(0x64) & 1;
        
        if (status != 1) {
            // set to 1? or need to wait?
            // might need to add more here 
            return false;
        }
        
        return true;
    }

    bool before_write() {
        // read bit 1 of Status Register byte
        int status = inb(0x64) & 2;
        
        if (status == 2) {
            // set to 0? or need to wait?
            // might need to add more here
            return false;
        }
        
        return true;
    }

    bool send_to_controller(int port, int command) {
        bool can_write = before_write();

	if (can_write) {
		outb(port, command);
		return true;
	}
		
	else {
		// don't know what to do
		return false;
	}
		
    }

    int read_from_port(int port) {
        bool can_read = before_read();

	if (!can_read) {
		return -1;
	}
		
	return inb(port);
    }

    /* 
        Both send_bytes and receiving bytes depend on whether
        we are using the first or second PS/2 port.
    */

    // sending bytes to a PS/2 device
    void send_bytes(int bytes) {
        // first PS/2 port
	timer.start_timer();
	bool can_write = before_write();

	while (!can_write && timer.get_time() < timer.TIMELIMIT) {
		can_write = before_write();
	}

	if (timer.get_time() >= timer.TIMELIMIT) {
		// error
	}

	else if (can_write) {
		// write to IO port 0x60
		send_to_controller(0x60, bytes);
	}

        
        // second PS/2 port
	send_to_controller(0x64, 0xD4); // 0xD4 is a command
	timer.start_timer();
	bool can_write = before_write();

	while (!can_write && timer.get_time() < timer.TIMELIMIT) {
		can_write = before_write();
	}

	if (timer.get_time() >= timer.TIMELIMIT) {
		// error
	}

	else if (can_write) {
		// write to IO port 0x60
		send_to_controller(0x60, bytes);
	}
    }
    
    // receiving bytes from a PS/2 device
    void receive_bytes() {
        // first PS/2 port - keyboard
	/*
		1. Enable IRQ1
		2. Read from 0x60 (don't need to check SR)
		3. Send EOI to interrupt controller
		4. Return EOI from interrupt handler
	*/


        // second PS/2 port - mouse
	/*
		1. Enable IRQ12
		2. Read from 0x60 (don't need to check SR)
		3. Send EOI to interrupt controller
		4. Return EOI from interrupt handler
	*/
    }
};

#endif
