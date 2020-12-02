#ifndef _PS2INTERFACE_H_
#define _PS2INTERFACE_H_

#include "machine.h"
#include "debug.h"
#include "heap.h"
#include "pit.h"

// need interrupt and atomic protections

// need a Timer class, or try to use jiffies (pit.h/cc)
// sample class
/*
class Timer {
	int32_t time;
public:
	int TIMELIMIT = 2000; // ms
	Timer () : time{0} {}

	void start_timer() {
		// start time
		//time = Pit::jiffies;
	}

	int32_t get_time() {
		//time = Pit::jiffies;
		return time;
	}
};
*/

class PS2INTERFACE {

	//Timer timer;

public:
    
	PS2INTERFACE () {}

	// SR = Status Register 

	// bit 0 of SR byte must be set before reading
	bool before_read() {
        // read bit 0 of SR byte
        int status = inb(0x64) & 1;
        
        if (status != 1) {
			/*
			timer->start_timer();

			while (status != 1 && timer->get_time() < Timer::TIMELIMIT) {
				status = inb(0x64) & 1;
			}

			if (status != 1) { */
				return false;
			//}
        }
        
        return true;
    }

	// bit 1 of SR byte must be cleared before writing
    bool before_write() {
        // read bit 1 of SR byte
        int status = inb(0x64) & 2;
        
        if (status == 2) {
			/*
            timer->start_timer();

			while (status != 1 && timer->get_time() < Timer::TIMELIMIT) {
				status = inb(0x64) & 2;
			}

			if (status == 2) {*/
				return false;
			//}
        }
        
        return true;
    }

	// send command to controller at given port
    bool send_to_controller(int port, int command) {
		// check that you can write to controller
        bool can_write = before_write();

		// if yes, send command to controller and
		// return success
		if (can_write) {
			outb(port, command);
			return true;
		}
			
		// else, timeout occurred before status was
		// changed; return failure
		return false;
    }

	// read 
    int read_from_port(int port) {
        bool can_read = before_read();

		if (!can_read) {
			return inb(port);
		}
		
		return -1;
    }

    /* 
        Both send_bytes and receiving bytes depend on whether
        we are using the first or second PS/2 port.
    */

    // sending bytes to PS/2 device
    void send_bytes(int bytes) {
        // first PS/2 port

		bool can_write = before_write();

		if (can_write) {
			// write to IO port 0x60
			send_to_controller(0x60, bytes);
		}

        
        // second PS/2 port

		send_to_controller(0x64, 0xD4); // 0xD4 is a command
		can_write = before_write();

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
