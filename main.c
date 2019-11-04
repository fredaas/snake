#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

typedef uint32_t pixel_t;

#define FPS 10
#define SLEEPTIME 1000 / FPS

/* Screen dimensions */
#define WIDTH (int)512
#define HEIGHT (int)512

#define T_WIDTH (int)(WIDTH / 2)
#define T_HEIGHT (int)(HEIGHT / 2)

#define SQUARE_SIZE 16

#define COLOR_BACKGROUND 0xffffffff
#define COLOR_APPLE 0xff5459ff
#define COLOR_SNAKE 0x404040ff

#define N_SQUARES (int)(T_WIDTH * T_HEIGHT / (SQUARE_SIZE * SQUARE_SIZE))
#define N_SQUARES_X (int)(T_WIDTH / SQUARE_SIZE)
#define N_SQUARES_Y (int)(T_HEIGHT / SQUARE_SIZE)

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture   = NULL;

pixel_t *pixels = NULL;

typedef struct Snake Snake;
typedef struct Square Square;

struct Square
{
    int x;
    int y;
} apple;

struct Snake
{
    int x[N_SQUARES];
    int y[N_SQUARES];
    int length;
    char lives;
} snake;

enum { LEFT, RIGHT, UP, DOWN };

int current_direction = RIGHT;
int next_direction    = RIGHT;

int apple_collision(void);
int snake_collision(void);
void draw_square(int x, int y, int size, pixel_t color);
void init_snake(int x, int y, int length);
void print_board_info(void);
void init_game(void);
void update_direction(void);
void update_snake(void);


/*------------------------------------------------------------------------------
 *
 * Snake
 *
 *----------------------------------------------------------------------------*/


void init_snake(int x, int y, int length)
{
    current_direction = RIGHT;
    next_direction    = RIGHT;

    for (int i = 0; i < length; i++)
    {
        snake.x[i] = x - i * SQUARE_SIZE;
        snake.y[i] = y;
    }
    snake.length = length;
}

void update_snake(void)
{
    update_direction();

    int *x = snake.x;
    int *y = snake.y;

    int x_prev[snake.length];
    int y_prev[snake.length];

    memcpy(x_prev, x, snake.length * sizeof(int));
    memcpy(y_prev, y, snake.length * sizeof(int));

    for (int i = 1; i < snake.length; i++)
    {
        x[i] = x_prev[i - 1];
        y[i] = y_prev[i - 1];
    }

    switch (current_direction)
    {
    case UP:
        y[0] -= SQUARE_SIZE;
        break;
    case DOWN:
        y[0] += SQUARE_SIZE;
        break;
    case RIGHT:
        x[0] += SQUARE_SIZE;
        break;
    case LEFT:
        x[0] -= SQUARE_SIZE;
        break;
    }

    for (int i = 0; i < snake.length; i++)
    {
        if (x[i] < 0)
            x[i] = T_WIDTH - SQUARE_SIZE;
        else if (x[i] >= T_WIDTH)
            x[i] = 0;
        else if (y[i] < 0)
            y[i] = T_HEIGHT - SQUARE_SIZE;
        else if (y[i] >= T_HEIGHT)
            y[i] = 0;
    }
}

void draw_square(int x, int y, int size, pixel_t color)
{
    for (int i = x; i < x + size; i++)
        for (int j = y; j < y + size; j++)
            pixels[T_WIDTH * j + i] = color;
}

void print_board_info(void)
{
    int world_x[N_SQUARES_X];
    int world_y[N_SQUARES_Y];

    for (int i = 0; i < N_SQUARES_Y; i++)
        world_y[i] = i * SQUARE_SIZE;
    for (int i = 0; i < N_SQUARES_X; i++)
        world_x[i] = i * SQUARE_SIZE;

    printf("N_SQUARES:   %d\n", N_SQUARES);
    printf("N_SQUARES_X: %d\n", N_SQUARES_X);
    printf("N_SQUARES_Y: %d\n", N_SQUARES_Y);
    printf("x: [ ");
    for (int i = 0; i < N_SQUARES_X; i++)
        printf("%d ", world_x[i]);
    printf("]\n");
    printf("y: [ ");
    for (int i = 0; i < N_SQUARES_Y; i++)
        printf("%d ", world_y[i]);
    printf("]\n");
}

int snake_collision(void)
{
    for (int i = 3; i < snake.length; i++)
        if ((snake.x[0] == snake.x[i]) && (snake.y[0] == snake.y[i]))
            return 1;
    return 0;
}

int apple_collision(void)
{
    return (apple.x == snake.x[0]) && (apple.y == snake.y[0]);
}

void init_game(void)
{
    init_snake(T_WIDTH / 2, T_HEIGHT / 2, 3);
    for (int i = 0; i < snake.length; i++)
        draw_square(snake.x[i], snake.y[i], SQUARE_SIZE, COLOR_SNAKE);

    memset(pixels, COLOR_BACKGROUND, T_WIDTH * T_HEIGHT * sizeof(pixel_t));

    apple.x = (rand() % N_SQUARES_X) * SQUARE_SIZE;
    apple.y = (rand() % N_SQUARES_Y) * SQUARE_SIZE;
    draw_square(apple.x, apple.y, SQUARE_SIZE, COLOR_APPLE);
}

void update_direction(void)
{
    /* NOP when direction is opposite to current direction */
    switch (next_direction)
    {
    case UP:
        if (current_direction == DOWN)
            return;
        break;
    case DOWN:
        if (current_direction == UP)
            return;
        break;
    case RIGHT:
        if (current_direction == LEFT)
            return;
        break;
    case LEFT:
        if (current_direction == RIGHT)
            return;
        break;
    }
    current_direction = next_direction;
}

int main(int argc, char **argv)
{
    Uint32 flags = SDL_WINDOW_HIDDEN;

    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        fprintf(stderr, "SDL_Init(SDL_INIT_VIDEO) failed: %s\n",
            SDL_GetError());
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(0, 0, flags, &window, &renderer) < 0)
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer() failed: %s\n",
            SDL_GetError());
        return 1;
    }

    pixels = (pixel_t *)malloc(T_WIDTH * T_HEIGHT * sizeof(pixel_t));
    memset(pixels, COLOR_BACKGROUND, T_WIDTH * T_HEIGHT * sizeof(pixel_t));

    /* Configure window */
    SDL_SetWindowTitle(window, argv[1]);
    SDL_SetWindowSize(window, WIDTH, HEIGHT);
    SDL_SetWindowPosition(
        window,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED
    );
    SDL_SetWindowTitle(window, "Snake");
    SDL_ShowWindow(window);

    /* Configure texture */
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING, T_WIDTH, T_HEIGHT);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    /* Configure renderer */
    SDL_SetRenderTarget(renderer, texture);

    /* Configure rect */
    SDL_Rect texture_rect;
    texture_rect.w = WIDTH;
    texture_rect.h = HEIGHT;
    texture_rect.x = 0;
    texture_rect.y = 0;

    init_game();

    SDL_bool done = SDL_FALSE;
    while (!done)
    {
        /* Clear snake tail */
        draw_square(snake.x[snake.length - 1], snake.y[snake.length - 1], SQUARE_SIZE,
            COLOR_BACKGROUND);

        /* Move snake */
        update_snake();

        /* Draw snake head */
        draw_square(snake.x[0], snake.y[0], SQUARE_SIZE, COLOR_SNAKE);

        if (apple_collision())
        {
            snake.x[snake.length] = snake.x[snake.length - 1];
            snake.y[snake.length] = snake.y[snake.length - 1];
            snake.length++;
            apple.x = (rand() % N_SQUARES_X) * SQUARE_SIZE;
            apple.y = (rand() % N_SQUARES_Y) * SQUARE_SIZE;
        }
        else if (snake_collision())
        {
            init_game();
        }

        /* Draw Apple */
        draw_square(apple.x, apple.y, SQUARE_SIZE, COLOR_APPLE);

        /* Render */
        SDL_UpdateTexture(texture, NULL, pixels, T_WIDTH * sizeof(pixel_t));
        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(SLEEPTIME);

        /* Handle I/O */
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_q:
                    done = SDL_TRUE;
                    break;
                case SDLK_w:
                    next_direction = UP;
                    break;
                case SDLK_a:
                    next_direction = LEFT;
                    break;
                case SDLK_s:
                    next_direction = DOWN;
                    break;
                case SDLK_d:
                    next_direction = RIGHT;
                    break;
                }
            }
        }
    }

    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
