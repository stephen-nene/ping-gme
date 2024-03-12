#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "video.h"
#include "header.h"
#define WIDTH 240
#define HEIGHT 160

#define MODE4 0x0004
#define BG2 0x0400
#define SHOW_BACK 0x10

volatile unsigned short* screen = (volatile unsigned short*)0x6000000;
volatile unsigned long* display_control = (volatile unsigned long*)0x4000000;
volatile unsigned short* palette = (volatile unsigned short*)0x5000000;
volatile unsigned short* front_buffer = (volatile unsigned short*)0x6000000;
volatile unsigned short* back_buffer = (volatile unsigned short*)0x600A000;
volatile unsigned short* buttons = (volatile unsigned short*)0x04000130;
volatile unsigned short* scanline_counter = (volatile unsigned short*)0x4000006;

// Declaration of next_palette_index
int next_palette_index = 0;

// Definition of button constants
#define BUTTON_UP (1 << 6)
#define BUTTON_DOWN (1 << 7)


const unsigned char fontdata_6x8[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x60, 0xFA, 0xFA, 0x60, 0x00, 0x00, 0x00}, // !
    // Add more characters here
};

void wait_vblank() {
    while (*scanline_counter < 160) { }
}

volatile unsigned short* flip_buffers(volatile unsigned short* buffer) {
    /* if the back buffer is up, return that */
    if(buffer == front_buffer) {
        /* clear back buffer bit and return back buffer pointer */
        *display_control &= ~SHOW_BACK;
        return back_buffer;
    } else {
        /* set back buffer bit and return front buffer */
        *display_control |= SHOW_BACK;
        return front_buffer;
    }
}


void draw_string(volatile unsigned short* buffer, int x, int y, char* str, unsigned short color) {
    int i;
    for (i = 0; i < strlen(str); i++) {
        char c = str[i];
        if (c >= ' ' && c <= '~') {
            int row, col;
            for (row = 0; row < 8; row++) {
                for (col = 0; col < 8; col++) {
                    if ((fontdata_6x8[c - 32][row] >> col) & 1) {
                        buffer[(y + row) * WIDTH + (x + col)] = color;
                    }
                }
            }
            x += 6; // Advance by 6 pixels for the next character
        }
    }
}

unsigned char add_color(unsigned char r, unsigned char g, unsigned char b) {
    unsigned short color = b << 10;
    color += g << 5;
    color += r;
    palette[next_palette_index] = color;
    next_palette_index++;
    return next_palette_index - 1;
}

void put_pixel(volatile unsigned short* buffer, int row, int col, unsigned char color) {
    unsigned short offset = (row * WIDTH + col) >> 1;
    unsigned short pixel = buffer[offset];
    if (col & 1) {
        buffer[offset] = (color << 8) | (pixel & 0x00FF);
    } else {
        buffer[offset] = (pixel & 0xFF00) | color;
    }
}

void draw_net(volatile unsigned short* buffer) {
    int i;
    for (i = 0; i < HEIGHT; i += 4) {
        put_pixel(buffer, i, WIDTH / 2, 0xFF);
    }
}

struct paddle {
    unsigned short x, y, width, height;
    unsigned char color;
};

struct ball {
    unsigned short x, y, size;
    short dx, dy;
    unsigned char color;
};

unsigned char button_pressed(unsigned short button) {
    /* and the button register with the button constant we want */
    unsigned short pressed = *buttons & button;

    /* if this value is zero, then it's not pressed */
    if (pressed == 0) {
        return 1;
    } else {
        return 0;
    }
}

void move_cp_paddle(struct paddle* p, bool up, bool down) {
    if (up && p->y > 0) {
        p->y--;
    }
    if (down && p->y + p->height < HEIGHT) {
        p->y++;
    }
}

void move_paddle(struct paddle* p) {
    if (button_pressed(BUTTON_DOWN)&&p->y > 0) {
        p->y += 1;
    }
    if (button_pressed(BUTTON_UP)&&p->y + p->height < HEIGHT) {
        p->y -= 1;
    }
}

void move_ball(struct ball* b) {
    b->x += b->dx;
    b->y += b->dy;
}

void draw_paddle(volatile unsigned short* buffer, struct paddle* p, unsigned char color) {
    unsigned short i, j;
    for (i = p->y; i < p->y + p->height; i++) {
        for (j = p->x; j < p->x + p->width; j++) {
            put_pixel(buffer, i, j, color);
        }
    }
}

void draw_ball(volatile unsigned short* buffer, struct ball* b, unsigned char color) {
    unsigned short i, j;
    for (i = b->y; i < b->y + b->size; i++) {
        for (j = b->x; j < b->x + b->size; j++) {
            put_pixel(buffer, i, j, color);
        }
    }
}

void clear_screen(volatile unsigned short* buffer, unsigned char color) {
    unsigned short row, col;
    for (row = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            put_pixel(buffer, row, col, color);
        }
    }
}

bool check_collision(struct paddle* p, struct ball* b) {
    return (b->x < p->x + p->width && b->x + b->size > p->x &&
            b->y < p->y + p->height && b->y + b->size > p->y);
}

void game_over(bool player_wins) {
    clear_screen(front_buffer, 0);
    char* message = player_wins ? "You Win!" : "You Lose!";
    int x = (WIDTH - strlen(message) * 8) / 2;
    int y = (HEIGHT - 8) / 2;
    draw_string(front_buffer, x, y, message, 0xFFFF);
    *display_control |= SHOW_BACK;
    while (1);
}

void update_screen(volatile unsigned short* buffer, unsigned char color, struct paddle* p, struct paddle* cp, struct ball* b) {
    draw_net(buffer);
    unsigned short i, j;
    for (i = p->y-3; i < p->y + p->height+3; i++) {
        for (j = p->x-3; j < p->x + p->width+3; j++) {
            put_pixel(buffer, i, j, color);
        }
    }
    for (i = cp->y-3; i < cp->y + cp->height+3; i++) {
        for (j = cp->x-3; j < cp->x + cp->width+3; j++) {
            put_pixel(buffer, i, j, color);
        }
    }
    for (i = b->y-3; i < b->y + b->size+3; i++) {
        for (j = b->x-3; j < b->x + b->size+3; j++) {
            put_pixel(buffer, i, j, color);
        }
    }
}

int main() {
    *display_control = MODE4 | BG2;

    struct paddle player_paddle = {20, 60, 5, 40};
    struct paddle computer_paddle = {WIDTH - 25, 60, 5, 40};
    struct ball ball = {WIDTH / 2, HEIGHT / 2, 3, -1, 1};

    unsigned char black = add_color(0, 0, 0);
    unsigned char white = add_color(31, 31, 31);

    volatile unsigned short* buffer = front_buffer;
    clear_screen(front_buffer, black);
    clear_screen(back_buffer, black);

    while (1) {
        update_screen(buffer, black, &player_paddle, &computer_paddle, &ball);

    	draw_net(front_buffer);
    	draw_paddle(front_buffer, &player_paddle, white);
    	draw_paddle(front_buffer, &computer_paddle, white);
    	draw_ball(front_buffer, &ball, white);

        move_paddle(&player_paddle);

        // AI movement for computer paddle
        if (ball.y < computer_paddle.y + computer_paddle.height / 2) {
            move_cp_paddle(&computer_paddle, true, false);
        } else {
            move_cp_paddle(&computer_paddle, false, true);
        }

        move_ball(&ball);

        if (ball.y <= 0 || ball.y + ball.size >= HEIGHT) {
            ball.dy = -ball.dy; // Change vertical direction upon hitting top or bottom
        }

        if (ball.x <= 0) {
            // Player loses
	    game_over(false);
        } else if (ball.x + ball.size >= WIDTH) {
            // Player wins
            game_over(true);
        }

        if (check_collision(&player_paddle, &ball) || check_collision(&computer_paddle, &ball)) {
            ball.dx = -ball.dx; // Change horizontal direction upon hitting paddle
        }

        wait_vblank();
        buffer = flip_buffers(buffer);
    }
}
