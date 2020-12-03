#include "libc.h"
#include "stdint.h"

void show_mouse_state(uint32_t state) {
    uint8_t byte0 = (state >> 0) & 0xFF;
    uint8_t byte1 = (state >> 8) & 0xFF;
    uint8_t byte2 = (state >> 16) & 0xFF;

    if ( byte0 & 0x04 ) {
        printf("*** middle button is enabled\n");
    } else {
        printf("*** middle button is disabled\n");
    }

    if ( byte0 & 0x02 ) {
        printf("*** right button is enabled\n");
    } else {
        printf("*** right button is disabled\n");
    }

    if ( byte0 & 0x01 ) {
        printf("*** left button is enabled\n");
    } else {
        printf("*** left button is disabled\n");
    }

    int x = byte1 - ((byte0 << 4) & 0x100);
    int y = byte2 - ((byte0 << 3) & 0x100);

    printf("*** cursor is currently at x: %d, y: %d\n", x, y);

    printf("\n");
}


int main(int argc, char** argv) {
    printf("Debug\n");
    shutdown();
    show_mouse_state(show_mouse());

    left_click_on();
    
    show_mouse_state(show_mouse());

    left_click_off();

    show_mouse_state(show_mouse());

    shutdown();

    return 0;
}
