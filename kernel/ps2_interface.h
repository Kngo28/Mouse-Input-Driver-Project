#ifndef _PS2INTERFACE_H_
#define _PS2INTERFACE_H_

#include "machine.h"
#include "debug.h"
#include "heap.h"
#include "pit.h"
#include "atomic.h"

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


class PS2INTERFACE {

	Timer timer;

public:
    
	PS2INTERFACE () : timer{} {}

    // SR = Status Register
    // bit 0 of SR = output buffer
    // bit 1 of SR = input buffer

    // bit 0 of SR byte must be set before reading
    bool before_read() {
        // read bit 0 of SR byte
        int status = inb(0x64) & 1;
        
        if (status != 1) {
            int time = 0;
            
            // keep polling bit 0 until set or timer runs out 
            while (status != 1 && timer.get_time() < timer.TIMELIMIT) {
                status = inb(0x64) & 1;
                time = timer.get_time();
            }

            if (status != 1) { 
                Debug::panic("Before Read Failed! Time Failed: %d ms\n", time);
                return false;
            }
        }
        
        return true;
    }

    // bit 1 of SR byte must be clear before writing
    bool before_write() {
        // read bit 1 of SR byte
        int status = inb(0x64) & 2;
        
        if (status == 2) {
            int time = 0;
            
	    // keep polling bit 1 until clear or timer runs out 
            while (status != 1 && timer.get_time() < timer.TIMELIMIT) {
                status = inb(0x64) & 2;
                time = timer.get_time();
            }
            
	    if (status == 2) {
                Debug::panic("Before Write Failed! Time Failed: %d ms\n", time);
                return false;
            }
        }
        
        return true;
    }

    // send command or bytes to controller at given port
    bool send_to_controller(int port, int message) {
        // check that input buffer is empty
        bool can_write = before_write();
        
	// if yes, send command or bytes to controller
        if (can_write) {
            outb(port, message);
            return true;
        }
            
        // else, timeout occurred before status was changed
        return false;
    } 

    // write bytes to a port
    bool send_to_port(int port, int bytes) {
        // check that input buffer is empty
        bool can_write = before_write();

        // if yes, send bytes to port
        if (can_write) {
            outb(port, bytes);
            return true;
        }

        // else, timeout occurred before status was changed
        return false;   
    }

    // read value from given port
    int read_from_port(int port) {
        // check that output buffer is empty
        bool can_read = before_read();
        
	// if yes, read from port
        if (can_read) {
            return inb(port);
        }
        
	// else, timeout occurred before status was changed
        return -1;
    }

    // sending a command or bytes to PS/2 device (first PS/2 port)
    void send_to_device(int message, bool command) {
        // check that input buffer is empty
        bool can_write = before_write();
        
	// if yes, write to IO port 0x60
        if (can_write) {
            // if sending a command to device, must send 0xD4 command
            // to controller first
            if (command) {
                send_to_controller(0x64, 0xD4);
            }

            send_to_controller(0x60, message);
        }
    }

    // receiving bytes from a PS/2 device
    int read_from_device() {
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
        return -1;
    }
};

#endif
