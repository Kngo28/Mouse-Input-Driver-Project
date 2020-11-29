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
/*
poll bit 0 (val == 1) of 0x64 to see if data in port 0x60 is mouse data
steps:
        1. Send “disable scanning” command 0xF5 to device
        2. Wait for 0xFA response from device (use timeout)
        3. Send “identify” command 0xF2 to device
        4. Wait for 0xFA response from device (use timeout)
        5. Wait for device to send up to 2 bytes of reply with timeout to determine
        when finished (e.g. only 1 byte sent)
                - Standard PS/2 Mouse sends 0x00 reply
        * Note: enable scanning at right time
        * Note: sending commands to mouse
                - send 0xD4 to port 0x64 (wait for bit 1 on port 0x64 beforehand)
                - send command to port 0x60
*/


extern "C" int receive_mouse_input(void);
/*
smp.cc line 41:
eoi_reg = (uint32_t*) (kConfig.localAPIC + 0xb0) ;

pseudo code for receiving bytes:

when IRQ1 occures:
int lapic??? = inb(0x60)
int EOI_reg = lapic + 0x0B0 (this is the offset of lapic registers to get EOI... need to get lapic somehow)
sent EOI to interrupt ctrller
outb(interrupt ctrller, EOI);
//return bytes from interrupt handler...

*/

#endif
