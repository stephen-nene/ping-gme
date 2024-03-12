#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define HEIGHT 20
#define WIDTH 60

int ballX, ballY;       // Ball position
int ballDirX, ballDirY; // Ball direction
int paddle1Y, paddle2Y; // Paddle positions

// Function to set terminal to non-canonical mode for reading single keypresses
void setTerminalMode() {
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Function to restore terminal mode to normal
void restoreTerminalMode() {
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Function to initialize game variables
void initGame() {
  ballX = WIDTH / 2;
  ballY = HEIGHT / 2;
  ballDirX = 2;
  ballDirY = 2;
  paddle1Y = HEIGHT / 2 - 2;
  paddle2Y = HEIGHT / 2 - 2;
}

// Function to draw the game board or menu
void drawBoard(int gamePaused) {
  system("clear"); // Clear the terminal screen

  if (gamePaused) {
    printf("Menu:\n");
    printf("1. Resume game\n");
    printf("2. Help \n");
    printf("3. Exit game\n");
  } else {
    printf("press q to pause the game\n");
    // Draw top wall
    for (int i = 0; i < WIDTH; i++)
      printf("*");
    printf("\n");

    // Draw middle section
    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
        if (j == 0 || j == WIDTH - 1)
          printf("*");
        else if (j == ballX && i == ballY)
          printf("O");
        else if (j == 1 && (i >= paddle1Y && i <= paddle1Y + 3))
          printf("|");
        else if (j == WIDTH - 2 && (i >= paddle2Y && i <= paddle2Y + 3))
          printf("|");
        else if (j == WIDTH / 2)
          printf("X");
        else
          printf(" ");
      }
      printf("\n");
    }

    // Draw bottom wall
    for (int i = 0; i < WIDTH; i++)
      printf("*");
    printf("\n");
  }
}

// Function to handle user input
void getInput(int *gamePaused) {
  char input;
  if (read(STDIN_FILENO, &input, 1) == 1) {
    if (*gamePaused) {
      switch (input) {
        case '1':
          *gamePaused = 0;
          break;
        case '3':
          restoreTerminalMode();
          exit(0);
          break;
      }
    } else {
      switch (input) {
        case 'w':
          if (paddle1Y > 1)
            paddle1Y--;
          break;
        case 's':
          if (paddle1Y + 4 < HEIGHT - 1)
            paddle1Y++;
          break;
        case 'i':
          if (paddle2Y > 1)
            paddle2Y--;
          break;
        case 'k':
          if (paddle2Y + 4 < HEIGHT - 1)
            paddle2Y++;
          break;
        case 'q':
          // Toggle game pause
          *gamePaused = !(*gamePaused);
          break;
      }
    }
  }
}

// Function to update the ball's position
void updateBall() {
  ballX += ballDirX;
  ballY += ballDirY;

  // Collision detection with walls
  if (ballY <= 0 || ballY >= HEIGHT - 1)
    ballDirY = -ballDirY;
  // Collision detection with paddles
  if (ballX == 2 && (ballY >= paddle1Y && ballY <= paddle1Y + 3))
    ballDirX = -ballDirX;
  if (ballX == WIDTH - 3 && (ballY >= paddle2Y && ballY <= paddle2Y + 3))
    ballDirX = -ballDirX;
  // Ball out of bounds
  if (ballX <= 0 || ballX >= WIDTH - 1)
    initGame();
}

int main() {
  setTerminalMode();
  initGame();

  int gamePaused = 0;

  while (1) {
    drawBoard(gamePaused);
    getInput(&gamePaused);
    if (!gamePaused){
      updateBall();
      // usleep(800); // Adjust game speed
    }
  }

  return 0;
}
