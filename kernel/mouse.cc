#include "mouse.h"
#include "stdint.h"
#include "idt.h"
#include "packet.h"

// from https://forum.osdev.org/viewtopic.php?t=10247

Timer* Mouse::timer = new Timer();
uint8_t Mouse::mouse_id = 0;

extern "C" void mouseHandler(uint32_t eax, uint32_t *frame) {

    MovementPacket* p = new MovementPacket(); //read from 0x60 in packet initialization

    if(p->is_valid()) {
        Debug::printf("x: %d   y: %d      left: %d   right: %d   middle: %d      ", p->get_xm(), p->get_ym(), p->get_left_on(), p->get_right_on(), p->get_middle_on());
        if(Mouse::mouse_id == 3) {
            Debug::printf("scroller: %d", p->get_scroll_state(3));
        } else if(Mouse::mouse_id == 4) {
            Debug::printf("4th: %d   5th: %d   scroller: %d", p->get_4th_on(), p->get_5th_on(), p->get_scroll_state(4));
        }
        Debug::printf("\n");
    }

    SMP::eoi_reg.exchange(0);
}


inline void Mouse::mouse_wait(uint8_t b) {
    if ( b == 0 ) { // read data
        while ( timer->get_time() < timer->TIMELIMIT ) {
            if ( (inb(0x64) & 1) == 1 ) {
                return;
            }
        }

        Debug::panic("Data Read Failed! Time Failed: %d ms\n", timer->get_time());

    } else { // send command
        while ( timer->get_time() < timer->TIMELIMIT ) {
            if ( (inb(0x64) & 2) == 0 ) {
                return;
            }
        }

        Debug::panic("Before Write Failed! Time Failed: %d ms\n", timer->get_time());

    }

    return;
}

void Mouse::mouse_write(uint8_t write_b) {
    mouse_wait(1);
    outb(0x64, 0xD4);

    mouse_wait(1);
    outb(0x60, write_b);
}

uint8_t Mouse::mouse_read_command() {
    mouse_wait(0);
    return inb(0x60);
}

uint32_t Mouse::mouse_read_packet() {
    mouse_wait(0);
    return inl(0x60);
}

void Mouse::set_sample_rate(uint32_t rate) {
    if ( Mouse::Debug && !((rate == 10) || (rate == 20) || (rate == 40) || (rate == 60) || (rate == 80) ||(rate == 100) || (rate == 200)) ) {
        Debug::panic("Invalid sample rate, rate = %d\n", rate);
    }

    uint32_t ack;

    mouse_write(0xF3);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
    }

    mouse_write(rate);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
    }
}

uint8_t Mouse::get_device_id() {
    uint32_t ack;

    mouse_write(0xF5);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA in get_device_id(), ack = 0x%x\n", ack);
    }

    mouse_write(0xF2);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA in get_device_id(), ack = 0x%x\n", ack);
    }

    return mouse_read_command();
}

void Mouse::set_mouse_mode(uint8_t m) {
    if ( m == 0 ) { // standard mouse with no extension
        return;
    } else if ( m == 1 ) { // Z-axis mouse
        set_sample_rate(200);
        set_sample_rate(100);
        set_sample_rate(80);

        Mouse::mouse_id = get_device_id();

        if ( Mouse::Debug && Mouse::mouse_id == 0 ) {
            Debug::panic("Mouse id should not be 0, id = %d\n", Mouse::mouse_id);
        }

    } else { // 4th and 5th button mouse
        set_sample_rate(200);
        set_sample_rate(100);
        set_sample_rate(80);

        Mouse::mouse_id = get_device_id();

        if ( Mouse::mouse_id == 3 ) {
            set_sample_rate(200);
            set_sample_rate(200);
            set_sample_rate(80);

            Mouse::mouse_id = get_device_id();

            if ( Mouse::Debug && Mouse::mouse_id != 4 ) {
            Debug::panic("Mouse id should be 4, id = %d\n", Mouse::mouse_id);
            }

        } else {
            if ( Mouse::Debug ) {
                Debug::panic("Mouse id should be 3, id = %d\n", Mouse::mouse_id);
            }
        }
    }
}

// if r = 0x00, resolution = 1 count/mm
// if r = 0x01, resolution = 2 count/mm
// if r = 0x02, resolution = 4 count/mm
// if r = 0x03, resolution = 8 count/mm
void Mouse::set_resolution(uint8_t r) {
    uint32_t ack;

    mouse_write(0xE8);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
    }

    mouse_write(r);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
    }
}

// if s = 0, scaling is 1:1
// if s = 1, scaling is 2:1
void Mouse::set_scaling(uint8_t s) {
    uint32_t ack;

    if ( s == 0 ) {
        mouse_write(0xE6);
        ack = mouse_read_command();
        if ( Mouse::Debug && ack != 0xFA ) {
            Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
        }
    } else if ( s == 1 ) {
        mouse_write(0xE7);
        ack = mouse_read_command();
        if ( Mouse::Debug && ack != 0xFA ) {
            Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
        }
    } else {
        if ( Mouse::Debug ) {
            Debug::panic("Invalid Input for setting scale, s = %d\n", s);
        }
    }

}

// followed the second example, scrolling mouse(Z-axis) from the following link: 
// http://users.utcluj.ro/~baruch/sie/labor/PS2/PS-2_Mouse_Interface.htm
void Mouse::init() {
    uint32_t status;
    uint32_t ack;

    mouse_wait(1);
    outb(0x64, 0xA8);

    mouse_wait(1);
    outb(0x64, 0x20);

    mouse_wait(0);
    status = inb(0x60) | 2;

    mouse_wait(1);
    outb(0x64, 0x60);

    mouse_wait(1);
    outb(0x60, status);

    // set default
    mouse_write(0xF6);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
    }

    // Z-axis mouse
    set_mouse_mode(1);

    // 8 count/mm
    set_resolution(0x03);

    // scaling 1:1
    set_scaling(0);

    // sample rate = 40
    set_sample_rate(40);

    // enable data report
    mouse_write(0xF4);
    ack = mouse_read_command();
    if ( Mouse::Debug && ack != 0xFA ) {
        Debug::panic("Acknowledge Not Equal to 0xFA, ack = 0x%x\n", ack);
    }

    IDT::interrupt(Mouse::VECTOR, (uint32_t)mouseHandler_);

}
