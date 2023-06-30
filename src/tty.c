#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#include "tty.h"

int TestModeConsole = 0;
struct termios oflags, nflags;

// For hiding terminal output
// Save terminal flags & setup the flag
// for hiding/showing input
// nflags :: new flags
// oflags :: old flage (original)
void tty_init() {
    tcgetattr(fileno(stdin), &oflags);
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;
}

int tty_hide_input() {
    // Set terminal state to hide input
    if (!isatty(fileno(stdin))) return 0;
    if (tcsetattr(fileno(stdin), TCSADRAIN, &nflags) != 0) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}
int tty_show_input() {
    // Set terminal state to show input
    if (!isatty(fileno(stdin))) return 0;
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}

int global_get_testmode() {
    return TestModeConsole;
}

void global_set_testmode(int value) {
    TestModeConsole = value;
}