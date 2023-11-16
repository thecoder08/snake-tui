#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}
#endif

typedef struct {
    int x;
    int y;
} Body;

Body snake[1920];
int length = 4;

void drawSnake() {
    printf("\e[102m");
    for (int i = 0; i < length; i++) {
        printf("\e[%d;%dH ", snake[i].y+1, snake[i].x+1);
    }
}

void moveSnake(Body direction) {
    for (int i = length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0].x += direction.x;
    snake[0].y += direction.y;
    if (snake[0].x == 80) {
        snake[0].x = 0;
    }
    if (snake[0].x == -1) {
        snake[0].x = 79;
    }
    if (snake[0].y == 24) {
        snake[0].y = 0;
    }
    if (snake[0].y == -1) {
        snake[0].y = 23;
    }
}

int snakeTouchingItself() {
    for (int i = 1; i < length; i++) {
        if ((snake[i].x == snake[0].x) && (snake[i].y == snake[0].y)) {
            return 1;
        }
    }
    return 0;
}

int main() {
    snake[0].x = 0; snake[0].y = 0;
    snake[1].x = 1; snake[1].y = 0;
    snake[2].x = 2; snake[2].y = 0;
    snake[3].x = 3; snake[3].y = 0;
    Body direction;
    direction.x = 0;
    direction.y = 1;
    Body apple;
    apple.x = rand() % 80;
    apple.y = rand() % 24;
    #ifndef WIN32
    set_conio_terminal_mode();
    #endif
    printf("\ec");
    while(1) {
        if (kbhit()) {
            char c = getch();
            if (c == '\e') {
                if (!kbhit()) { // we need this to prevent arrow keys from exiting the game
                    #ifndef WIN32
                    reset_terminal_mode();
                    #endif
                    printf("\ecThanks for playing! Score: %d\n", length);
                    return 0;
                }
            }
            if (c == 'w' || c == 'W') {
                direction.y = -1;
                direction.x = 0;
            }
            if (c == 's' || c == 'S') {
                direction.y = 1;
                direction.x = 0;
            }
            if (c == 'a' || c == 'A') {
                direction.y = 0;
                direction.x = -1;
            }
            if (c == 'd' || c == 'D') {
                direction.y = 0;
                direction.x = 1;
            }
        }
        if ((snake[0].x == apple.x) && (snake[0].y == apple.y)) {
            snake[length].x = snake[length - 1].x - direction.x;
            snake[length].y = snake[length - 1].y - direction.y;
            length++;
            apple.x = rand() % 80;
            apple.y = rand() % 24;
        }
        if (snakeTouchingItself()) {
            #ifndef WIN32
            reset_terminal_mode();
            #endif
            printf("\ecYou died! Score: %d\n", length);
            return 0;
        }
        moveSnake(direction);
        printf("\ec");
        printf("\e[101m");
        printf("\e[%d;%dH ", apple.y+1, apple.x+1);
        drawSnake();
        printf("\e[1;1H");
        fflush(stdout);
        usleep(100000);
    }
}
