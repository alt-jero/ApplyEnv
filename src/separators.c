#include <stdlib.h>
#include <string.h>

#include "separators.h"
#include "testlib.h"
#include "chartypes.h"
#include "parselib.h"

// Requirements:
// 1. Starting at the beginning of input, check each character
// to ensure that it represents whitespace (space / tab)
// 2. The first non-whitespace character denotes the end.
// 3. The input contains further material to be parsed by
// a subsequent parser, thus any successfully parsed characters
// must be removed from the start of the string.
//
// I: input - input buffer (&string)
// I: inputLength - max length to be parsed; excludes terminator
void ignoreSpaces(char** inputPointer, size_t inputLength) {
    // Check if input is NULL or inputLength is 0
    if (*inputPointer == NULL || inputLength == 0)
        return;
        
    p_skipWhile(inputPointer, inputLength, is_whitespace);
}

void skipNewlines(char** inputPointer, size_t inputLength) {
    // Check if input is NULL or inputLength is 0
    if (*inputPointer == NULL || inputLength == 0)
        return;
        
    p_skipWhile(inputPointer, inputLength, is_newline);
}

// Requirements:
// 1. Check if the first character of input is '='.
// 2. If so, continue. If not, stop.
// 3. The input contains further material to be parsed by
// a subsequent parser, thus any successfully parsed characters
// must be removed from the start of the string.
//
// I: input - input buffer (&string)
// I: inputLength - max length to be parsed; excludes terminator
// O: int - 1 if found equals, 0 if no equals.
bool expect_equals(char** inputPointer, size_t inputLength) {
    char* input = *inputPointer;
    // Check if input is NULL or inputLength is 0
    if (input == NULL || inputLength == 0)
        return false;

    if(input[0] == '=') {
        // Ensure that the next parser to be called starts
        // at the right position
        (*inputPointer) += 1;
        return true;
    }
    else return false;
}

void skipComment(char** inputPointer, size_t inputLength) {
    // Check if input is NULL or inputLength is 0
    if (*inputPointer == NULL || inputLength == 0)
        return;
    
    char* input = (*inputPointer);

    // Check if first symbol is #
    if (input[0] != '#') return;
    p_skipWhile(inputPointer, inputLength, not_newline);
}

void test_ignoreSpaces() {
    describeSection("void ignoreSpaces(char** inputPointer, size_t inputLength);");
    char* input = "   foo=bar";
    char* badInput = "foo=bar";
    char* testInput = input;
    char* testBadInput = badInput;
    ignoreSpaces(&testInput, strlen(input));
    ignoreSpaces(&testBadInput, strlen(badInput));

    describe("Increments pointer on success");
    assert_str_equal(testInput, "foo=bar");

    describe("Leaves pointer on fail");
    assert_str_equal(testBadInput, "foo=bar");
}

void test_skipNewlines() {
    describeSection("void skipNewlines(char** inputPointer, size_t inputLength);");

    char* input = "\n\n   foo=bar";
    char* badInput = "foo=bar";
    char* testInput = input;
    char* testBadInput = badInput;
    skipNewlines(&testInput, strlen(input));
    skipNewlines(&testBadInput, strlen(badInput));

    describe("Increments pointer on success");
    assert_str_equal(testInput, "   foo=bar");

    describe("Leaves pointer on fail");
    assert_str_equal(testBadInput, "foo=bar");
}

void test_expect_equals() {
    describeSection("bool expect_equals(char** inputPointer, size_t inputLength);");
    char* input = "=bar";
    char* testInput = input;
    char* badInput = "bar";
    char* testBadInput = badInput;
    int result = expect_equals(&testInput, strlen(input));
    int badResult = expect_equals(&testBadInput, strlen(badInput));

    describe("Increments pointer on success");
    assert_str_equal(testInput, "bar");

    describe("Leaves pointer on fail");
    assert_str_equal(testBadInput, "bar");

    describe("Returns true on success");
    assert_bool_equal(result, true);
    
    describe("Returns false on fail");
    assert_bool_equal(badResult, false);
}

void test_skipComment() {
    describeSection("void skipComment(char** inputPointer, size_t inputLength);");
    char* input = "# this is a wonderful comment \n";
    char* badInput = "foo=bar";
    char* testInput = input;
    char* testBadInput = badInput;
    skipComment(&testInput, strlen(input));
    skipComment(&testBadInput, strlen(badInput));

    describe("Increments pointer on success");
    assert_str_equal(testInput, "\n");

    describe("Leaves pointer on fail");
    assert_str_equal(testBadInput, "foo=bar");
}

void test_separators() {
    describeFile("separators.c");
    test_ignoreSpaces();
    test_expect_equals();
    test_skipComment();
}