#include <stdlib.h>

#include "chartypes.h"

bool charIsOneOf(char c, const char* str) {
    if (str == NULL) {
        // Handle invalid string
        return false;
    }

    while (*str != '\0') {
        if (*str == c) {
            // Character found in the string
            return true;
        }
        str++; // Move to the next character in the string
    }

    // Character not found in the string
    return false;
}

bool charNotOneOf(char c, const char* str) {
    if (str == NULL) {
        // Handle invalid string
        return false;
    }

    while (*str != '\0') {
        if (*str == c) {
            // Character found in the string
            return false;
        }
        str++; // Move to the next character in the string
    }

    // Character not found in the string
    return true;
}

bool is_newline(char testChar) {
    return testChar == '\n';
}

bool not_newline(char testChar) {
    return testChar != '\n';
}

bool is_whitespace(char testChar) {
    return charIsOneOf(testChar, " \t");
}