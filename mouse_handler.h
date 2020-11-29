#ifndef _MOUSE_HANDLER_H_
#define _MOUSE_HANDLER_H_

#include "stdint.h"

class MOUSE_HANDLER {
    public:
    static const uint32_t IRQ = 12;
    static const uint32_t VECTOR = 60;
};

extern "C" void mouse_init();

extern "C" int detect_ps2_type(void);
//poll bit 0 (val == 1) of 0x64 to see if data in port 0x60 is mouse data



extern "C" int receive_mouse_input(void);
/*
pseudo code for receiving bytes:

when IRQ1 occures:
int lapic??? = inb(0x60)
int EOI_reg = lapic + 0x0B0 (this is the offset of lapic registers to get EOI... need to get lapic somehow)
sent EOI to interrupt ctrller
outb(interrupt ctrller, EOI);
//return bytes from interrupt handler...

*/

#endif