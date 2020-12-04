#include "mouse_handler.h"
#include "machine.h"
#include "idt.h"
#include "debug.h"
#include "ps2_interface.h"

// borrowed from osdev
void PIC_sendEOI(unsigned char irq) {
	if(irq >= 8)
		outb(0xa0,0x20);
	outb(0x20,0x20);
}

MouseState* current_mouse_state = new MouseState();

extern "C" void mouseHandler(uint32_t eax, uint32_t *frame) {

    uint32_t *userEsp = (uint32_t*)frame[3];
    uint32_t userPC = frame[0];

    Debug::printf("*** mouse movement call#%d, userEsp = %x, userPC = %x\n",eax, userEsp, userPC);

    Packet* p = new MovementPacket(); //read from 0x60 in packet initialization

    switch(eax) {
        case 14: // void middle_click_on(void)
            current_mouse_state->enable_bm();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 15: // void middle_click_off(void)
            current_mouse_state->disable_bm();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 16: // void right_click_on(void)
            current_mouse_state->enable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 17: // void right_click_off(void)
            current_mouse_state->disable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 18: // void left_click_on(void)
            current_mouse_state->disable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 19: // void left_click_off(void)
            current_mouse_state->disable_bl();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 20: // void move_mouse(int xm, int ym)
            {
                int xm = (int)userEsp[1];
                int ym = (int)userEsp[2];

                p->set_xm(xm);
                p->set_ym(ym);

                current_mouse_state->add_x(xm);
                current_mouse_state->add_y(ym);
                outb(0x60, (current_mouse_state->get_packet() & 0x0000CF) | (p->get_packet() & ~0x0000CF) );
            }
            break;

        case 21: // uint32_t show_mouse(void)
            //return current_mouse_state->get_packet();
            Debug::printf("");
	    break;

        default:
            Debug::printf("Not a proper mouse behavior\n");
	    break;
    }

    SMP::eoi_reg.exchange(0);
    //return -1;
}

void mouse_init() {
    Debug::printf("touched\n");

    outb(0x64, 0xa8);

    outb(0x64, 0x20);
    int status = inb(0x60);
    status = status | 2;
    outb(0x64, 0x60);
    outb(0x60, status);

    outb(0x64, 0xD4);
    outb(0x60, 0xF6);
    inb(0x60);

    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    inb(0x60);

    //MouseState* current_mouse_state = new MouseState();
    //current_mouse_state->reset();
    IDT::interrupt(MOUSE_HANDLER::VECTOR, (uint32_t)mouseHandler_);
}

int receive_mouse_input(void) {
    Debug::printf("success!\n");
    PIC_sendEOI(MOUSE_HANDLER::IRQ);
    return 0;
}

int detect_ps2_type() {
    PS2INTERFACE* ps2 = new PS2INTERFACE();

    // step 1: disable scanning on device
    ps2->send_to_device(0xF5, true);

    // step 2: wait for 0xFA response from device
    int response = ps2->read_from_device();
    if (response != 0xFA) {
        return -1;
    }

    // step 3: identify command on device
    ps2->send_to_device(0xF2, true);

    // step 4: wait for 0xFA response from device
    response = ps2->read_from_device();
    if (response != 0xFA) {
        return -1;
    }

    // step 5: wait for 0x00 (PS/2 mouseID) response from device
    int mouseID = ps2->read_from_device();
    if (response != 0xFA) {
        return -1;
    }

    return mouseID;
}
