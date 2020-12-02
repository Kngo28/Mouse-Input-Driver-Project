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

extern "C" int mouseHandler(uint32_t eax, uint32_t *frame) {

    uint32_t *userEsp = (uint32_t*)frame[3];
    // uint32_t userPC = frame[0];

    //Debug::printf("*** mouse movement call#%d, userEsp = %x, userPC = %x\n",eax, userEsp, userPC);

    Packet* p = new Packet();

    switch(eax) {
        case 0: // void middle_click_on(void)
            current_mouse_state->enable_bm();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            outb(current_mouse_state->get_packet() & 0x0000CF, 0x60);
            break;

        case 1: // void middle_click_off(void)
            current_mouse_state->disable_bm();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            outb(current_mouse_state->get_packet() & 0x0000CF, 0x60);
            break;

        case 2: // void right_click_on(void)
            current_mouse_state->enable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            outb(current_mouse_state->get_packet() & 0x0000CF, 0x60);
            break;

        case 3: // void right_click_off(void)
            current_mouse_state->disable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            outb(current_mouse_state->get_packet() & 0x0000CF, 0x60);
            break;

        case 4: // void left_click_on(void)
            current_mouse_state->disable_br();
            outb(0x60, current_mouse_state->get_packet() & 0x0000CF);
            outb(current_mouse_state->get_packet() & 0x0000CF, 0x60);
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

    PS2INTERFACE* ps2 = new PS2INTERFACE();

    //step 3:
    ps2->send_to_controller(0x64, 0xAD);
    ps2->send_to_controller(0x64, 0xA7);
    //outb(0x64, 0xAD);
    //outb(0x64, 0xA7);

    //step 4:
    //ps2->read_from_port(0x60);
    inb(0x60);
    //might have to read (inb) w/o testing bit 0...

    //step 5:
    ps2->send_to_controller(0x64, 0x20);
    //outb(0x64, 0x20);
    int status = ps2->read_from_port(0x60);
    // int status = inb(0x60);  // bit 5 is set (therefore it is a duel channel PS2 controller)       0x71:  0111 0001
    Debug::printf("status before: 0x%x\n", status);                                      //       
    status = (status & 0xDF) | 2;                                                              // 0x53:  0101 0011                                                                   // 0x30:  0011 0000
    Debug::printf("status after: 0x%x\n", status);
    //ps2->send_to_controller(0x64, 0x6000 | status);
    outb(0x64, 0x6000 | status); //next byte (34) is being sent by command (0x60)
    //outb(0x60, status);

    //step 6:
    // send_to_controller(0x64, 0xAA);
    outb(0x64, 0xAA);
    // read_from_port(0x60);
    inb(0x60); // flush
    // int test = read_from_port(0x60);
    int test = inb(0x60);
    if(test == 0x55) {
        Debug::printf("Controller Self Test PASSED with 0x%x\n", test);
    } else {
       Debug::printf("Controller Self Test FAILED with 0x%x\n", test);
    }

    //step 7: skip
    // send_to_controller(0x64, 0xa8);
    outb(0x64, 0xa8);
    // status = read_from_port(0x60);
    status = inb(0x60);
    Debug::printf("status2: 0x%x\n", status);

    //step 8:
    // send_to_controller(0x64, 0xab);
    outb(0x64, 0xAB);  // test port 1
    // int interface_test = read_from_port(0x60);
    int interface_test = inb(0x60);
    if(interface_test == 0) {
        Debug::printf("pass interface test\n");
    } else {
        Debug::panic("bad interface test\n");
    }
    // send_to_controller(0x64, 0xa9);
    outb(0x64, 0xA9);  // test port 2
    // interface_test = read_from_port(0x60);
    interface_test = inb(0x60);
    if(interface_test == 0) {
        Debug::printf("pass interface test\n");
    } else {
        Debug::panic("bad interface test\n");
    }

    //step 9:
    // send_to_controller(0x64, 0xae);
    outb(0x64, 0xAE);
    // status = read_from_port(0x60);
    status = inb(0x60);
    status = status | 1;  // set bit 0 to enable interrupts on PS2 1
    // send_to_controller(0x64, 0x6000 | status);
    outb(0x64, 0x6000 | status);
    //Debug::printf("status3: %d\n", status);

    // send_to_controller(0x64, 0xa8);
    outb(0x64, 0xA8);
    // status = read_from_port(0x60);
    status = inb(0x60);
    status = status | 2;  // set bit 1 to enable interrupts on PS2 2
    // send_to_controller(0x64, 0x6000 | status);
    outb(0x64, 0x6000 | status);

    //step 10: RESET...
    // send_to_controller(0x64, 0xff);
    outb(0x64, 0xFF);
    // int acknowledge = read_from_port(0x60);
    int acknowledge = inb(0x60);
    if (acknowledge == 0xFA) {
        Debug::printf("reset success\n");
    } else if (acknowledge == 0xFC) {
        Debug::printf("reset failure\n");
    } else {
        Debug::printf("no device present\n");
    }

    MouseState* current_mouse_state = new MouseState();
    current_mouse_state->reset();
    //MouseState* current_mouse_state = new MouseState();
    IDT::interrupt(MOUSE_HANDLER::VECTOR, (uint32_t)mouseHandler_);
}

int receive_mouse_input(void) {
    Debug::printf("success!\n");
    PIC_sendEOI(MOUSE_HANDLER::IRQ);
    return 0;
}
