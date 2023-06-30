#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parselib.h"

void p_skipWhile(char** inputPointer, size_t inputLength, bool (*predicate)(const char testChar)) {
    char* input = *inputPointer;

    if (input == NULL || inputLength == 0)
        return;

    size_t position = 0;

    while (predicate(input[position])) {
        if(position >= inputLength) break;
        position++;
    }

    (*inputPointer) += position;
}