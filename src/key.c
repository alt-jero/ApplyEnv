#include <stdlib.h>
#include <string.h>

#include "key.h"
#include "testlib.h"

// Requirements:
// 1. Parse out a key, starting at the beginning of a string
// and continuing while valid characters are encountered.
// 2. The first invalid character denotes the end of the key.
// 3. The input contains further material to be parsed by
// a subsequent parser, thus any successfully parsed characters
// must be removed from the start of the string.
//
// I: input - input buffer (&string)
// I: inputLength - max length to be parsed; excludes terminator
// O: the parsed key
#define MAX_ENV_KEY_SIZE 256
char* parse_key(char** inputPointer, size_t inputLength) {
    char* input = *inputPointer;
    // Check if input is NULL or inputLength is 0
    if (input == NULL || inputLength == 0)
        return NULL;

    size_t actualKeySize = (inputLength <= MAX_ENV_KEY_SIZE)
        ? inputLength
        : MAX_ENV_KEY_SIZE;

    // Allocate memory for key (including null terminator)
    char* key = malloc((actualKeySize + 1) * sizeof(char));
    if(key == NULL) return NULL;

    size_t position = 0;

    // Key must not start with a number.
    if((input[position] >= 'a' && input[position] <= 'z') ||
           (input[position] >= 'A' && input[position] <= 'Z') ||
           (input[position] == '_')) {
        key[position] = input[position];
        position++;
    } else {
        free(key);
        return NULL;
    }

    // Key can contain letters, underscores, and digits.
    // The first invalid character denotes the end of the key.
    while ((input[position] >= 'a' && input[position] <= 'z') ||
           (input[position] >= 'A' && input[position] <= 'Z') ||
           (input[position] == '_') || (input[position] >= '0' && input[position] <= '9')) {
        if(position >= MAX_ENV_KEY_SIZE) break;
        key[position] = input[position];
        position++;
    }

    key[position] = '\0'; // Add null terminator

    // If key has exceeded max length, fast forward past the rest of it. (Truncate key)
    // Allows parsing to continue.
    while ((input[position] >= 'a' && input[position] <= 'z') ||
           (input[position] >= 'A' && input[position] <= 'Z') ||
           (input[position] == '_') || (input[position] >= '0' && input[position] <= '9')) {
        position++;
    }

    // Ensure that the next parser to be called starts
    // at the right position
    (*inputPointer) += position;

    return key;
}

void test_parse_key() {
    describeSection("char* parse_key(char** inputPointer, size_t inputLength);");
    describe("A valid key");
    char* c1 = "aTestKey=foo";
    char* r1 = parse_key(&c1, strlen(c1));
    assert_str_equal(r1, "aTestKey");
    assert_str_equal(c1, "=foo");
    free(r1);

    describe("Key exceeds max length");
    char* case2 = createString(257);
    char* t2 = createString(256);
    char* c2 = case2;
    char* r2 = parse_key(&c2, strlen(c2));
    assert_str_equal(r2, t2);
    assert_str_equal(c2, "");
    assert_size_t_equal(strlen(r2), strlen(t2));
    free(t2);
    free(case2);
    free(r2);

    describe("Key may not start with a digit");
    char* c3 = "1key=foo";
    char* r3 = parse_key(&c3, strlen(c3));
    assert_is_null(r3);
    assert_str_equal(c3, "1key=foo");

    describe("Key may not be NULL");
    char* c4 = NULL;
    char* r4 = parse_key(&c4, 1);
    assert_is_null(r4);

    describe("inputLength may not be zero");
    char* c5 = "";
    char* r5 = parse_key(&c5, 0);
    assert_is_null(r5);
}

void test_key() {
    describeFile("key.c");
    test_parse_key();
}