#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define CANVAS_WIDTH 40
#define CANVAS_HEIGHT 30

#define BRACKGROUND_COLOR 0x232323
#define SNAKE_COLOR 0xff2323
#define FRUIT_COLOR 0x23ff23

void get_rgb_from_color(int color, int *r, int *g, int *b) {
  *r = (color >> 16) & 0xff;
  *g = (color >> 8)  & 0xff;
  *b = (color >> 0) & 0xff;
}

typedef enum Dir {
  DIR_UP = 0,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT,
} Dir;

typedef struct Segment {
  int x;
  int y;
} Segment;

Segment *new_segment(int x, int y) {
  Segment *seg = malloc(sizeof(Segment));

  seg->x = x;
  seg->y = y;

  return seg;
}

typedef struct Snake {
  Dir dir;
  Segment segs[CANVAS_HEIGHT * CANVAS_HEIGHT];
  size_t size;
} Snake;

typedef struct Fruit {
  int x;
  int y;
} Fruit;

Snake snake = {0};
Fruit fruit = {0};
int score = 0;

void snake_update() {
  Segment current = snake.segs[0];

  for (size_t i = 1; i < snake.size; i++) {
    Segment next = snake.segs[i];

    snake.segs[i] = current;

    current = next;
  }

  snake.segs[snake.size++] = current;

  switch (snake.dir) {
    case DIR_UP:
      snake.segs[0].y--;
      break;
    case DIR_DOWN:
      snake.segs[0].y++;
      break;
    case DIR_LEFT:
      snake.segs[0].x--;
      break;
    case DIR_RIGHT:
      snake.segs[0].x++;
      break;
  }

  if (snake.segs[0].x == fruit.x && snake.segs[0].y == fruit.y) {
    fruit.x = rand() % (CANVAS_WIDTH - 1);
    fruit.y = rand() % (CANVAS_HEIGHT - 1);

    score++;

    return;
  }

  snake.size--;
}

void redraw_canvas() {
  printf("\033[2J");
  printf("\033[H");

  for (size_t y = 0; y < CANVAS_HEIGHT; y++) {
    for (size_t x = 0; x < CANVAS_WIDTH; x++) {
      int r, g, b;

      int is_snake = 0;
      for (size_t i = 0; i < snake.size; i++) {
        Segment seg = snake.segs[i];

        if (seg.x == (int)x && seg.y == (int)y) {
          get_rgb_from_color(SNAKE_COLOR, &r, &g, &b);
          printf("\033[48;2;%d;%d;%dm  ", r, g, b);

          is_snake = 1;
          break;
        }
      }
      if (is_snake) continue;


      if (fruit.x == (int)x && fruit.y == (int)y) {
        get_rgb_from_color(FRUIT_COLOR, &r, &g, &b);
        printf("\033[48;2;%d;%d;%dm  ", r, g, b);

        continue;
      }

      get_rgb_from_color(BRACKGROUND_COLOR, &r, &g, &b);
      printf("\033[48;2;%d;%d;%dm  ", r, g, b);
    }
    printf("\n");
  }

  printf("\033[m");

  printf("Press 'q' to exit. Score: %d\n", score);
}

int main(void) {
  srand(time(NULL));

  struct termios term;
  tcgetattr(fileno(stdin), &term);

  term.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(fileno(stdin), 0, &term);

  fcntl(0, F_SETFL, O_NONBLOCK);

  snake.segs[snake.size++] = (Segment){.x = 20, .y = 20};
  fruit.x = rand() % (CANVAS_WIDTH - 1);
  fruit.y = rand() % (CANVAS_HEIGHT - 1);

  int quit = 0;

  while (!quit) {
    int x = getchar();

    switch (x) {
      case 'q':
        quit = 1;
        break;
      case 'w':
        snake.dir = DIR_UP;
        break;
      case 's':
        snake.dir = DIR_DOWN;
        break;
      case 'a':
        snake.dir = DIR_LEFT;
        break;
      case 'd':
        snake.dir = DIR_RIGHT;
        break;
    }

    snake_update();
    redraw_canvas();

    usleep(200 * 1000);
  }

  term.c_lflag |= ECHO; 
  term.c_lflag |= ICANON;
  tcsetattr(fileno(stdin), 0, &term);
  return 0;
}
