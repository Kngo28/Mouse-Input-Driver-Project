#include "mouse_handler.h"
#include "machine.h"
#include "idt.h"
#include "debug.h"

// borrowed from osdev
void PIC_sendEOI(unsigned char irq) {
	if(irq >= 8)
		outb(0xa0,0x20);
	outb(0x20,0x20);
}

void mouse_init() {
    Debug::printf("touched\n");
    IDT::interrupt(MOUSE_HANDLER::VECTOR, (uint32_t)receive_mouse_input_);
}

int receive_mouse_input(void) {
    Debug::printf("success!\n");
    PIC_sendEOI(MOUSE_HANDLER::IRQ);
    return 0;
}