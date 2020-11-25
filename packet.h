#ifndef _PACKET_H_
#define _PACKET_H_

#include "stdint.h"

#define MOVE_LIMIT 0x100

class Packet {
    uint8_t byte0; // yo xo ys xs ao bm br bl
    uint8_t byte1; // xm
    uint8_t byte2; // ym
    uint8_t byte3; // not going to use it, but maybe for a padding

public:
    Packet() {
        byte0 = 0x08;
        byte1 = 0x00;
        byte2 = 0x00;
        byte3 = 0x00;
    }

    void reset() {
        byte0 = 0x08;
        byte1 = 0x00;
        byte2 = 0x00;
        byte3 = 0x00;
    }

    inline void set_yo() {
        byte0 |= 0x80;
    }

    inline void set_xo() {
        byte0 |= 0x40;
    }
    
    inline void set_ys() {
        byte0 |= 0x20;
    }

    inline void set_xs() {
        byte0 |= 0x10;
    }

    inline void set_bm() {
        byte0 |= 0x04;
    }

    inline void set_br() {
        byte0 |= 0x02;
    }

    inline void set_bl() {
        byte0 |= 0x01;
    }

    void set_ym(int ym) {
        if (ym >= MOVE_LIMIT) {
            set_yo();
            byte2 = MOVE_LIMIT - 1;
        } else if (ym < -MOVE_LIMIT) {
            set_yo();
            set_ys();
            byte2 = 0x00;
        } else if (ym >= 0) {
            byte2 = ym;
        } else {
            set_ys();
            byte2 = ym + 0x100;
        }
    }

    void set_xm(int xm) {
        if (xm >= MOVE_LIMIT) {
            set_xo();
            byte1 = MOVE_LIMIT - 1;
        } else if (xm < -MOVE_LIMIT) {
            set_xo();
            set_xs();
            byte1 = 0x00;
        } else if (xm >= 0) {
            byte1 = xm;
        } else {
            set_xs();
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

#endif