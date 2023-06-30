#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "testlib.h"
#include "tty.h"

int testsPerformed = 0;
int testsPassed = 0;
int testsFailed = 0;
int testsPending = 0;

int currentTestAssertionFails = 0;
int currentTestToBeImplemented = 0;

void assert_not_implemented() {
    _TTY_E(TTY_RED, "   - TODO: Code not yet implemented\n");
    currentTestToBeImplemented++;
}

void assert_str_equal(char* actual, char* expected) {
    if(strcmp(actual, expected) != 0) {
        _TTY_E(TTY_RED, "   - Test Failed: char* '%s' should equal '%s'\n", actual, expected);
        currentTestAssertionFails++;
    }
}

void assert_size_t_equal(size_t actual, size_t expected) {
    if(actual != expected) {
        _TTY_E(TTY_RED, "   - Test Failed: size_t '%zu' should equal '%zu'\n", actual, expected);
        currentTestAssertionFails++;
    }
}

void assert_int_equal(int actual, int expected) {
    if(actual != expected) {
        _TTY_E(TTY_RED, "   - Test Failed: int '%d' should equal '%d'\n", actual, expected);
        currentTestAssertionFails++;
    }
}

void assert_bool_equal(bool actual, bool expected) {
    if(actual != expected) {
        char* sActual = actual ? "true" : "false";
        char* sExpected = expected ? "true" : "false";
        _TTY_E(TTY_RED, "   - Test Failed: bool '%s' should equal '%s'\n", sActual , sExpected);
        currentTestAssertionFails++;
    }
}

void assert_is_null(char* actual) {
    if(actual != NULL) {
        _TTY_E(TTY_RED, "   - Test Failed: char* '%s' should be NULL\n", actual);
        currentTestAssertionFails++;
    }
}

void testlib_init() {
    global_set_testmode(1);
    _TTY(TTY_CYAN, "System Test\n");
    _TTY_E(TTY_CYAN, "===========\n");
}

void _increment_test_count() {
    // Overall Test Result
    bool failed = currentTestAssertionFails > 0;
    bool pending = currentTestToBeImplemented > 0;

    currentTestAssertionFails = 0;
    currentTestToBeImplemented = 0;

    // Failed should take priority
    if(failed) {
        testsFailed ++;
        return;
    }
    // Pending is still not successful
    if(pending) {
        testsPending ++;
        return;
    }
    // Otherwise the test passed.
    testsPassed ++;
}

int testlib_finalize() {
    _increment_test_count();
    printf("\n");
    _TTY(TTY_CYAN, "# Results\n");
    if(testsPassed == testsPerformed) {
        _TTY(TTY_GREEN, " - All tests: PASS\n");
        _TTY(TTY_CYAN, " - Total: %d\n", testsPerformed);
        return 0;
    }
    if(testsPassed == 0) _TTY(TTY_GREEN, " - Passed: NONE\n");
    else _TTY(TTY_GREEN, " - Passed: %d\n", testsPassed);
    if(testsFailed == 0) _TTY(TTY_RED, " - Failed: NONE\n");
    else _TTY(TTY_RED, " - Failed: %d\n", testsFailed);
    if(testsPending > 0) _TTY(TTY_YELLOW, " - Pending: %d\n", testsPending);
    _TTY(TTY_CYAN, " - Total: %d\n", testsPerformed);
    return 1;
}

void describeFile(char* description) {
    printf("\n");
    _TTY(TTY_CYAN, "# File: %s\n", description);
}

void describeSection(char* description) {
    printf("\n");
    _TTY(TTY_YELLOW, "## Section: %s\n", description);
}

void describe(char* description) {
    _TTY(TTY_BLUE, " - Test: %s\n", description);
    testsPerformed ++;

    if(testsPerformed == 1) return;
    _increment_test_count();
}

char* createString(int n) {
    char* str = malloc((n + 1) * sizeof(char));  // Allocate memory for the string

    if (str != NULL) {
        for (int i = 0; i < n; i++) {
            str[i] = 'A';  // Fill the string with the letter 'A'
        }

        str[n] = '\0';  // Add null terminator at the end
    }

    return str;
}