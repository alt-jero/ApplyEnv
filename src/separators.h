#include <stdbool.h>
void test_separators();
void ignoreSpaces(char** inputPointer, size_t inputLength);
void skipNewlines(char** inputPointer, size_t inputLength);
bool expect_equals(char** inputPointer, size_t inputLength);
void skipComment(char** inputPointer, size_t inputLength);
