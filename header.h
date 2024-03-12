#include "video.h"

#define MODE3   3
#define BG2_ENABLE (1 << 10)
#define BACKBUFFER (1 << 4)

#define WIDTH 240
#define HEIGHT 160

volatile unsigned short int* front_buffer = (volatile unsigned short int*)0x6000000;
volatile unsigned short int* back_buffer = (volatile unsigned short int*)0x600A000;
volatile unsigned short int* buffer;

#define SetMode(mode) REG_DISPCNT = (mode)
#define WaitVBlank() while (*(volatile unsigned short int*)0x4000006 >= 160); while (*(volatile unsigned short int*)0x4000006 < 160)

#define REG_DISPCNT *(volatile unsigned int*)0x4000000

void SetColor(unsigned short int* buffer, int row, int col, unsigned short int color) {
    buffer[row * WIDTH + col] = color;
}

void DrawRectangle(unsigned short int* buffer, int x, int y, int width, int height, unsigned short int color) {
    for (int row = y; row < y + height; row++) {
        for (int col = x; col < x + width; col++) {
            SetColor(buffer, row, col, color);
        }
    }
}

class start_game() {
    SetMode(MODE3 | BG2_ENABLE | BACKBUFFER);
    REG_DISPCNT = MODE3 | BG2_ENABLE | BACKBUFFER;

    buffer = front_buffer;

    while (1) {
        WaitVBlank();

        if (REG_DISPCNT & BACKBUFFER) {
            REG_DISPCNT &= ~BACKBUFFER;
            buffer = front_buffer;
        } else {
            REG_DISPCNT |= BACKBUFFER;
            buffer = back_buffer;
        }

        DrawRectangle(buffer, 20, 60, 5, 40, 0x7FFF);  // Player Paddle
        DrawRectangle(buffer, WIDTH - 25, 60, 5, 40, 0x7FFF);  // Computer Paddle
        DrawRectangle(buffer, WIDTH / 2, HEIGHT / 2, 3, 3, 0x7FFF);  // Ball
    }

    return 0;
}
