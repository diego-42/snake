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

typedef enum State {
  STATE_PLAYING = 0,
  STATE_QUIT,
  STATE_GAME_OVER,
} State;

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

State game_update() {
  int x = getchar();

  switch (x) {
    case 'q':
      return STATE_QUIT;
      break;
    case 'w':
      if (snake.dir != DIR_DOWN) snake.dir = DIR_UP;
      break;
    case 's':
      if (snake.dir != DIR_UP) snake.dir = DIR_DOWN;
      break;
    case 'a':
      if (snake.dir != DIR_RIGHT) snake.dir = DIR_LEFT;
      break;
    case 'd':
      if (snake.dir != DIR_LEFT) snake.dir = DIR_RIGHT;
      break;
  }

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

  Segment head = snake.segs[0];

  for (size_t i = 1; i < snake.size; i++) {
    Segment body = snake.segs[i];

    if (head.x == body.x && head.y == body.y) {
      return STATE_GAME_OVER;
    }
  }

  if (snake.segs[0].x == fruit.x && snake.segs[0].y == fruit.y) {
    fruit.x = rand() % (CANVAS_WIDTH - 1);
    fruit.y = rand() % (CANVAS_HEIGHT - 1);

    score++;

  } else snake.size--;
  
  return STATE_PLAYING;
}

void game_render(State state) {
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

  switch (state) {
    case STATE_PLAYING:
      printf("Press 'q' to exit. Score: %d\n", score);
      break;
    case STATE_QUIT:
      printf("Quiting...\n");
      break;
    case STATE_GAME_OVER:
      printf("Game over. Total score: %d\n", score);
  }
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
    State state = game_update();

    if (state == STATE_QUIT || state == STATE_GAME_OVER) {
      quit = 1;
    }

    game_render(state);

    usleep(200 * 1000);
  }

  term.c_lflag |= ECHO; 
  term.c_lflag |= ICANON;
  tcsetattr(fileno(stdin), 0, &term);
  return 0;
}
