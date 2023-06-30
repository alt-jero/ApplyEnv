#include <stdio.h>

void tty_init();
int tty_hide_input();
int tty_show_input();

int global_get_testmode();
void global_set_testmode(int value);

#define TTY_BLACK 30
#define TTY_RED 31
#define TTY_GREEN 32
#define TTY_YELLOW 33
#define TTY_BLUE 34
#define TTY_PURPLE 35
#define TTY_CYAN 36
#define TTY_WHITE 37

#define TTY_SET_COLOR(color_code) \
    printf("\033[0;%dm", color_code)

#define TTY_SET_ERROR_COLOR(color_code) \
    fprintf(stderr, "\033[0;%dm", color_code)

#define TTY_RESET_COLOR() \
    printf("\033[0m")

#define TTY_RESET_ERROR_COLOR() \
    fprintf(stderr, "\033[0m")

#define _TTY(color_code, format, ...) \
    do { \
        TTY_SET_COLOR(color_code); \
        printf(format, ##__VA_ARGS__); \
        TTY_RESET_COLOR(); \
    } while (0)

#define _TTY_E(color_code, format, ...) \
    do { \
        TTY_SET_ERROR_COLOR(color_code); \
        fprintf(stderr, format, ##__VA_ARGS__); \
        TTY_RESET_ERROR_COLOR(); \
    } while (0)

#define TTY(color_code, format, ...) \
    do { \
        if(!global_get_testmode()) {  \
            _TTY(color_code, format, ##__VA_ARGS__); \
        } \
    } while (0) 

#define TTY_E(color_code, format, ...) \
    do { \
        if(!global_get_testmode()) {  \
            _TTY_E(color_code, format, ##__VA_ARGS__); \
        } \
    } while (0) 