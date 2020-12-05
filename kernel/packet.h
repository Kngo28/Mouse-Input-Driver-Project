#ifndef _PACKET_H_
#define _PACKET_H_

#include "stdint.h"
#include "machine.h"
#include "debug.h"

#define MOVE_LIMIT 0x100

// abstract class that is for packets, including the following:
// 1. status packet
// 2. mouse movement data packet with no extension (using only first three bytes)
// 3. mouse movement data packet with Z-axis
// 4. mouse movement data packet with 5 buttons
class Packet {
protected:
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;

public:
    Packet() {
        uint32_t raw_data = (uint32_t) inl(0x60); //read from port 0x60 after interrupt in mouse_handler
        byte0 = (uint8_t)(raw_data & 0x000000FF);
        byte1 = (uint8_t)((raw_data & 0x0000FF00) >> 8);
        byte2 = (uint8_t)((raw_data & 0x00FF0000) >> 16);
        byte3 = (uint8_t)((raw_data & 0xFF000000) >> 24);
    }

    //~Packet() = 0;

    bool get_bit_from_byte0(int b) {
        ASSERT( (0 <= b) && (b < 8) );
        return byte0 & (1 << b);
    }

    uint32_t get_packet() {
        uint32_t result = 0x00000000;
        result = result
               | byte0
               | (byte1 << 8)
               | (byte2 << 16)
               | (byte3 << 24);
        return result;
    }

};

// mouse movement data packet with no extension
class MovementPacket : public Packet {
    // byte0 represents yo xo ys xs ao bm br bl
    // byte1 represents xm
    // byte2 represents ym
public:
    MovementPacket() : Packet() {
        //if ( (byte0 & 0x08) == 0 ) { //test 3rd bit = 0 (Should always be 1)
        if(!is_valid()) {
            //Debug::panic("Bad movement packet... throw it out\n");
        }
    }

    uint8_t is_valid() {
        return (byte0 & 0x08) != 0;
    }

    int get_left_on() {
        return byte0 & 0x1;
    }

    int get_right_on() {
        return (byte0 >> 1) & 0x1;
    }

    int get_middle_on() {
        return (byte0 >> 2) & 0x1;
    }

    int get_4th_on() {
        return (byte3 >> 4) & 0x1;
    }

    int get_5th_on() {
        return (byte3 >> 5) & 0x1;
    }

    inline int get_ym() {
        return (int)byte2 - ((byte0 << 3) & 0x100);
    }

    inline int get_xm() {
        return (int)byte1 - ((byte0 << 4) & 0x100);
    }

    int get_scroll_state(int id) {
        int result = 0;
        if(id == 3)  result = (int)byte3;
        if(id == 4)  result = (int)(byte3 & 0xF) - (byte3 & 0x8);
        return result;
    }

};


class ZaxisPacket : public MovementPacket {
    // first 4 bits of byte3 represent zm
public:
    // from http://users.utcluj.ro/~baruch/sie/labor/PS2/PS-2_Mouse_Interface.htm:
    // Z Movement is a 2's complement number that represents 
    // the scrolling wheel's movement since the last data report.
    // Valid values are in the range of -8 to +7. This means 
    // the number is actually represented only by the least significant four bits; 
    // the upper four bits act only as sign extension bits.
    inline int get_zm() {
        return ((int)byte3 & 0x07) - ((int)byte3 & 0x08);
    }
};


class FifthPacket : public ZaxisPacket {
public:
    FifthPacket() : ZaxisPacket() {
        if ( (byte3 & 0xC0) != 0 ) { // test 6th and 7th bit of byte3 = 0 (Should always be 0)
            Debug::panic("Bad movement packet for 5th button... throw it out\n");
        }
    }
};


class StatusPacket : public Packet {
    // byte0 represents the following:
    // bit0: right button
    // bit1: middle button
    // bit2: left button
    // bit3: always 0
    // bit4: scaling
    // bit5: enable
    // bit6: mode
    // bit7: always 0
    // byte1 represents resolution
    // byte2 represents sample rate
public:
    StatusPacket() : Packet() {
        if ( ((byte0 & 0x80) != 0) || ((byte0 & 0x08) != 0) ) { // test 3th and 7th bit of byte0 = 0 (both should always be 0)
            Debug::panic("Bad status packet... throw it out\n");
        }
    }
};

#endif
