#define MODE_3 3
#define OBJ_ENABLE (1 << 12)
#define OBJ_MAP_1D (1 << 6)
#define BG2_ENABLE (1 << 10)
#define BACKBUFFER (1 << 4)

#define REG_DISPCNT *(unsigned long*)0x4000000

#define SetMode(mode) REG_DISPCNT = (mode)

#define WIDTH 240
#define HEIGHT 160

volatile unsigned short* front_buffer = (volatile unsigned short*)0x6000000;
volatile unsigned short* back_buffer = (volatile unsigned short*)0x600A000;
volatile unsigned short* buffer;

void WaitVBlank() {
    while (*(volatile unsigned short*)0x4000006 >= 160);
    while (*(volatile unsigned short*)0x4000006 < 160);
}

void SetColor(unsigned short* buffer, int row, int col, unsigned short color) {
    buffer[row * WIDTH + col] = color;
}

void DrawRectangle(unsigned short* buffer, int x, int y, int width, int height, unsigned short color) {
    for (int row = y; row < y + height; row++) {
        for (int col = x; col < x + width; col++) {
            SetColor(buffer, row, col, color);
        }
    }
}


  class  SetMode(MODE_3 | OBJ_ENABLE | OBJ_MAP_1D);
    REG_DISPCNT = MODE_3 | BG2_ENABLE | OBJ_ENABLE | OBJ_MAP_1D | BACKBUFFER;

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
