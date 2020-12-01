#include "mouse_handler.h"
#include "machine.h"
#include "idt.h"
#include "debug.h"
#include "packet.h"

// borrowed from osdev
void PIC_sendEOI(unsigned char irq) {
	if(irq >= 8)
		outb(0xa0,0x20);
	outb(0x20,0x20);
}

extern "C" int mouseHandler(uint32_t eax, uint32_t *frame) {

    uint32_t *userEsp = (uint32_t*)frame[3];
    // uint32_t userPC = frame[0];

    //Debug::printf("*** mouse movement call#%d, userEsp = %x, userPC = %x\n",eax, userEsp, userPC);

    Packet* p = new Packet();

    switch(eax) {
        case 0: // void middle_click_on(void)
            current_mouse_state->enable_bm();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 1: // void middle_click_off(void)
            current_mouse_state->disable_bm();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 2: // void right_click_on(void)
            current_mouse_state->enable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 3: // void right_click_off(void)
            current_mouse_state->disable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 4: // void left_click_on(void)
            current_mouse_state->disable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 5: // void left_click_off(void)
            current_mouse_state->disable_bl();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            break;

        case 6: // void move_mouse(int xm, int ym)
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

        case 7: // uint32_t show_mouse(void)
            return current_mouse_state->get_packet();

        default:
            Debug::panic("Not a proper mouse behavior\n");
    }

    return -1;
}

void mouse_init() {
    Debug::printf("touched\n");
    MouseState* current_mouse_state = new MouseState();
    current_mouse_state->reset();
    IDT::interrupt(MOUSE_HANDLER::VECTOR, (uint32_t)mouseHandler_);
}

int receive_mouse_input(void) {
    Debug::printf("success!\n");
    PIC_sendEOI(MOUSE_HANDLER::IRQ);
    return 0;
}
