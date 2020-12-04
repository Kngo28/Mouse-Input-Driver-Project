#ifndef _PACKET_H_
#define _PACKET_H_

#include "stdint.h"
#include "machine.h"

#define MOVE_LIMIT 0x100

class Packet {
    uint8_t byte0; // yo xo ys xs ao bm br bl
    uint8_t byte1; // xm
    uint8_t byte2; // ym
    uint8_t byte3; // not going to use it, but maybe for a padding

public:
    Packet() { //comes in as 32 bit data
        uint32_t raw_data = (uint32_t) inl(0x60); //read from port 0x60 after interrupt in mouse_handler
        byte0 = (uint8_t)(raw_data & 0x000000FF);
        byte1 = (uint8_t)((raw_data & 0x0000FF00) >> 8);
        byte2 = (uint8_t)((raw_data & 0x00FF0000) >> 16);
        byte3 = (uint8_t)((raw_data & 0xFF000000) >> 24);
        if((byte0 & 08) >> 3 == 0) { //test 3rd bit = 0 (Should always be 1)
            Debug::panic("Bad packet... throw it out\n");
        }
        // byte0 = 0x08;
        // byte1 = 0x00;
        // byte2 = 0x00;
        // byte3 = 0x00;
    }

    void reset() {
        byte0 = 0x08;
        byte1 = 0x00;
        byte2 = 0x00;
        byte3 = 0x00;
    }

    inline void enable_yo() {
        byte0 |= 0x80;
    }

    inline void disable_yo() {
        byte0 &= ~0x80;
    }

    inline void enable_xo() {
        byte0 |= 0x40;
    }

    inline void disable_xo() {
        byte0 &= ~0x40;
    }
    
    inline void enable_ys() {
        byte0 |= 0x20;
    }

    inline void disable_ys() {
        byte0 &= ~0x20;
    }

    inline void enable_xs() {
        byte0 |= 0x10;
    }

    inline void disable_xs() {
        byte0 &= ~0x10;
    }

    inline void enable_bm() {
        byte0 |= 0x04;
    }

    inline void disable_bm() {
        byte0 &= ~0x04;
    }

    inline void enable_br() {
        byte0 |= 0x02;
    }

    inline void disable_br() {
        byte0 &= ~0x02;
    }

    inline void enable_bl() {
        byte0 |= 0x01;
    }

    inline void disable_bl() {
        byte0 &= ~0x01;
    }

    void set_ym(int ym) {
        if (ym >= MOVE_LIMIT) {
            enable_yo();
            byte2 = MOVE_LIMIT - 1;
        } else if (ym < -MOVE_LIMIT) {
            enable_yo();
            enable_ys();
            byte2 = 0x00;
        } else if (ym >= 0) {
            byte2 = ym;
        } else {
            enable_ys();
            byte2 = ym + 0x100;
        }
    }

    void set_byte0(int byte0) {

    }

    void set_xm(int xm) {
        if (xm >= MOVE_LIMIT) {
            enable_xo();
            byte1 = MOVE_LIMIT - 1;
        } else if (xm < -MOVE_LIMIT) {
            enable_xo();
            enable_xs();
            byte1 = 0x00;
        } else if (xm >= 0) {
            byte1 = xm;
        } else {
            enable_xs();
            byte1 = xm + 0x100;
        }
    }

    inline int get_ym() {
        return (int)byte2 - ((byte0 << 4) & 0x100);
    }

    inline int get_xm() {
        return (int)byte1 - ((byte0 << 4) & 0x100);
    }

    uint32_t get_packet() {
        uint32_t result = 0;
        result = result
               | byte0
               | (byte1 << 8)
               | (byte2 << 16)
               | (byte3 << 24);
        return result;
    }

};


class MouseState : public Packet {
public:
    inline void add_x(int xm) {
        set_xm(get_xm() + xm);
    }

    inline void add_y(int ym) {
        set_ym(get_ym() + ym);
    }
};

#endif
