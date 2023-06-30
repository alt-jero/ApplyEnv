// file: main.c

// Standard imports
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Parsing (& testing)
#include "key.h"
#include "separators.h"
#include "value.h"
#include "keyvalue.h"

// General Use
#include "tty.h"
#include "help.h"

// Testing
#include "testlib.h"


#define INITIAL_BUFFER_SIZE 1024

int binaryPathPosition = 1;
bool hide_input = false;
bool parser_v2 = false;

int test_all() {
    testlib_init();

    test_key();
    test_value();
    test_separators();
    test_keyvalue();

    return testlib_finalize();
}

void parse_args(int argc, char *argv[]) {
    // Iterate through command-line arguments
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') break;
        binaryPathPosition ++;
        if (strcmp(argv[i], "--help") == 0) {
            // Print help information and exit
            exit(print_help_info(argv[0]));
        }
        if (strcmp(argv[i], "--systemTest") == 0) {
            // Print help information and exit
            exit(test_all());
        }
        if (strcmp(argv[i], "--hide") == 0) {
            // Disable input hiding
            hide_input = true;
            continue;
        }
        if (strcmp(argv[i], "--parser-v2") == 0) {
            // Enable Beta parser
            parser_v2 = true;
            continue;
        }
    }
}

char* read_stdin() {
    // Set an initial buffer size
    size_t buffer_size = 1024;
    // Allocate memory for the buffer
    char* buffer = (char*)malloc(buffer_size * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    size_t total_size = 0;  // Total size of the input read so far
    size_t bytes_read;      // Number of bytes read in each iteration

    // Read the input in chunks until the end is reached
    while ((bytes_read = fread(buffer + total_size, sizeof(char), buffer_size - total_size, stdin)) > 0) {
        total_size += bytes_read;
        // If the buffer is full, resize it
        if (total_size == buffer_size) {
            buffer_size *= 2;  // Double the buffer size
            buffer = (char*)realloc(buffer, buffer_size * sizeof(char));
            if (buffer == NULL) {
                fprintf(stderr, "Memory reallocation failed.\n");
                exit(1);
            }
        }
    }

    // Null-terminate the buffer
    buffer[total_size] = '\0';

    return buffer;
}

int count_newlines(char* from, size_t until) {
    int line = 1;
    for(size_t i = 0; i < until; i ++) {
        if(from[i] == '\n') line++;
    }
    return line;
}

void v2_parse_input() {
    char* inputBuffer = read_stdin();

    char* buffer = inputBuffer;
    ssize_t totalSize = strlen(inputBuffer);
    
    char* prevBuffer = buffer;
    while((buffer - inputBuffer) < totalSize) {
        prevBuffer = buffer;
        ignoreSpaces(&buffer, totalSize - (buffer - inputBuffer));
        skipComment(&buffer, totalSize - (buffer - inputBuffer));
        skipNewlines(&buffer, totalSize - (buffer - inputBuffer));
        parse_keyvalue(&buffer, totalSize - (buffer - inputBuffer));
        if(prevBuffer == buffer) {
            // Nothing parsed this round; Warn and bail.
            int line = count_newlines(inputBuffer, (buffer - inputBuffer));
            TTY_E(TTY_RED, "Error (line %d): Got stuck while parsing input. Exiting infinite loop!\n", line);
            TTY_E(TTY_CYAN, "Hint: Variable names must start with a letter or an underscore.\n");

            exit(1);
        }
    }

    free(inputBuffer);
    // exit(0); // remove me
}


// void simple_parse_input() {
//     // Read key-value pairs from stdin
//     char buffer[256];
//     bool skip = false;
//     while (fgets(buffer, sizeof(buffer), stdin)) {
//         // Check for blank line
//         if (strcmp(buffer, "\n") == 0)
//             break;

//         // Line is too long. Balk.
//         if(strstr(buffer, "\n") == NULL) {
//             fprintf(stderr, "Error: Environment variable lines may not exceed %lu characters total.\n", sizeof(buffer) - 1);
//             exit(1);
//         }

//         // Extract key and value from the input line
//         char *key = strtok(buffer, "=");
//         char *value = strtok(NULL, "\n");

//         // Handle input that was larger than 256 characters
//         // - Truncate key at 255
//         // - 
//         if (strlen(key) >= 255) {
//             if( !fgets(buffer, sizeof(buffer), stdin) ) break;
//             char *keyremainder = strtok(buffer, "=");
//             char *value = strtok(NULL, "\n");
//         }

//         // if(key && value)
//         //     printf("=======TEST: Key(%lu), Value(%lu)\n", strlen(key), strlen(value));
//         // else if(key)
//         // printf("=======TEST: Key(%lu), Value(X)\n", strlen(key));

//         // Set the environment variable
//         // Allow NULL value to function as unset.
//         if (key && value)
//             setenv(key, value, 1);
//         if (key && !value)
//             unsetenv(key);
//     }
// }

int main(int argc, char *argv[]) {
    tty_init();

    // Check if the binary path is provided
    if (argc < 2) 
        return print_help_info(argv[0]);

    parse_args(argc, argv);

    // Set terminal state to hide input
    if(hide_input && tty_hide_input() != 0) return -1;

    v2_parse_input();

    // Reset terminal state before continuing
    if(hide_input && tty_show_input() != 0) return -1;

    // Execute the binary with the provided path and environment
    execvp(argv[binaryPathPosition], argv + binaryPathPosition);

    // If execvp returns, an error occurred
    perror("execvp");
    return 1;
}

