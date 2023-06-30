#include <stdlib.h>
#include <string.h>

#include "testlib.h"
#include "key.h"
#include "value.h"
#include "separators.h"
#include "chartypes.h"

void ignoreExport(char** tailPointer, size_t tailLength) {
    char* input = (*tailPointer);
    if(input == NULL || tailLength == 0) return;
    char* compare = "export";
    if(tailLength < 6) return; // Too short for "exports";
    for(size_t i = 0; i < 6; i++) {
        if(compare[i] != input[i]) return;
    }
    (*tailPointer) +=6;
}

void parse_keyvalue(char** tailPointer, size_t tailLength) {
    char* input = (*tailPointer);
    char* key;
    char* value;

    ignoreSpaces(&input, strlen(input));
    ignoreExport(&input, strlen(input));
    ignoreSpaces(&input, strlen(input));
    key = parse_key(&input, tailLength);
    if(key == NULL) return; // Not a valid key-value pair.
    ignoreSpaces(&input, strlen(input));
    if(!expect_equals(&input, strlen(input))) return; // Missing equals sign
    ignoreSpaces(&input, strlen(input));
    value = parse_value(&input, strlen(input));
    // Don't check for missing value here. Missing value => unsetenv();
    ignoreSpaces(&input, strlen(input));
    skipComment(&input, strlen(input));
    skipNewlines(&input, strlen(input));

    if (key && value)
        setenv(key, value, 1);
    if (key && !value)
        unsetenv(key);

    free(key);
    if(value) free(value);
    (*tailPointer) = input;
}

void test_parse_keyvalue() {
    describeSection("void parse_keyvalue(char** tailPointer, size_t tailLength);");
    char* test;
    char* result;

    describe("sets env var (nospace bare eof)");
    test = "FOO=BAR";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "BAR");
    assert_str_equal(test, "");

    describe("sets env var (space bare eof)");
    test = "FOO = BAZ";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "BAZ");
    assert_str_equal(test, "");

    describe("sets env var (space bare trailingspace eof)");
    test = "FOO = BOOP   ";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "BOOP");
    assert_str_equal(test, "");

    describe("sets env var (space bare trailingspace comment eof)");
    test = "FOO = PLACA   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "PLACA");
    assert_str_equal(test, "");

    describe("sets env var (space q-single trailingspace comment eof)");
    test = "FOO = 'ZAPPA'   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "ZAPPA");
    assert_str_equal(test, "");

    describe("sets env var (space q-double trailingspace comment eof)");
    test = "FOO = \"PAPA\"   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "PAPA");
    assert_str_equal(test, "");

    describe("sets env var (space h-single trailingspace comment eof)");
    test = "FOO = '''\nROKKA\n'''   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "ROKKA");
    assert_str_equal(test, "");

    describe("sets env var (space h-double trailingspace comment eof)");
    test = "FOO = \"\"\"\nQWERT\n\"\"\"   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("FOO");
    assert_str_equal(result, "QWERT");
    assert_str_equal(test, "");

    describe("interpolation double (space h-double trailingspace comment eof)");
    test = "BAR = \"\"\"\nZAZA${FOO}\n\"\"\"   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("BAR");
    assert_str_equal(result, "ZAZAQWERT");
    assert_str_equal(test, "");

    describe("No interpolation single (space h-single trailingspace comment eof)");
    test = "BAR = '''\nZAZA${FOO}\n'''   # hello world";
    parse_keyvalue(&test, strlen(test));
    result = getenv("BAR");
    assert_str_equal(result, "ZAZA${FOO}");
    assert_str_equal(test, "");

    describe("Escapes & interpolation based on escapes (space h-double trailingspace comment lf)");
    test = "BAR = \"\"\"\nMIMI\\u0024{FOO}\n\"\"\"   # hello world\n";
    parse_keyvalue(&test, strlen(test));
    result = getenv("BAR");
    assert_str_equal(result, "MIMIQWERT");
    assert_str_equal(test, "");

    describe("Ignores leading export (space h-double trailingspace comment lf)");
    test = "  export   BAR = \"\"\"\nVODE\\u0024{FOO}\n\"\"\"   # hello world\n";
    parse_keyvalue(&test, strlen(test));
    result = getenv("BAR");
    assert_str_equal(result, "VODEQWERT");
    assert_str_equal(test, "");
}

void test_keyvalue() {
    describeFile("keyvalue.c");
    test_parse_keyvalue();
}