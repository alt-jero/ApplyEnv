#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "value.h"
#include "separators.h"
#include "chartypes.h"
#include "testlib.h"
#include "tty.h"

char* parse_bare_value(char** inputPointer, size_t inputLength) {
    char* input = *inputPointer;
    // Check if input is NULL or inputLength is 0
    if (input == NULL || inputLength == 0)
        return NULL;
    
    size_t position = 0;
    // While char not one of [# or \n], take.
    while(input[position] != '\0' && charNotOneOf(input[position], "#\n")) {
        position ++;
    }
    // Get back to the last valid character
    position --;
    // Trim trailing whitespace.
    while(input[position] != '\0' && charIsOneOf(input[position], " \t")) {
        position --;
    }
    // Get back to the first whitespace character
    position ++;
    // Allocate space for return
    char* result = malloc((position + 1) * sizeof(char));
    strncpy(result, input, position);
    result[position] = '\0'; // Terminator

    (*inputPointer) += position;
    return result;
}

char* parse_quoted_value(char** inputPointer, size_t inputLength, bool* interpolate) {
    char* input = *inputPointer;
    // Check if input is NULL or inputLength is 0
    if (input == NULL || inputLength == 0)
        return NULL;

    // Make sure we have a quoted value
    if (input[0] != '\'' && input[0] != '"') return NULL;
    
    // Define our quote type
    char quote = input[0];
    char* qlf = quote == '"' ? "\"\n" : "'\n";

    input++; // skip the opening quote
    
    size_t position = 0;
    // While char is not a single quote or newline, take. Newline indicates invalid.
    while(input[position] != '\0' && (charNotOneOf(input[position], qlf) || input[position-1] == '\\')) {
        position ++;
    }
    // Quoted must end in same quote.
    if(input[position] != quote) return NULL;
    // Allocate space for return
    char* result = malloc((position + 1) * sizeof(char));
    strncpy(result, input, position);
    result[position] = '\0'; // Terminator

    (*inputPointer) += position + 2; // Drop both quotes for tail
    (*interpolate) = quote == '"'; // Flag double for interpolation
    return result;
}

char* parse_heredoc_value(char** inputPointer, size_t inputLength, bool* interpolate) {
    char* input = *inputPointer;
    char* originalInput = *inputPointer;
    // Check if input is NULL or inputLength is less than 3 for initial checks
    if (input == NULL || inputLength < 3)
        return NULL;

    // Make sure we have a quoted value
    if (input[0] != '\'' && input[0] != '"') return NULL;
    
    // Define our quote type
    char quote = input[0];
    
    // Make sure we have a quoted heredoc
    if (input[0] != quote) return NULL;
    if (input[1] != quote) return NULL;
    if (input[2] != quote) return NULL;


    input += 3; // skip the opening quotes
    ignoreSpaces(&input, strlen(input));
    // Check for unfinished line - Invalid heredoc
    if (input[0] == '\n') input ++; // Skip first newline
    if (input[0] == '\0') return NULL; // If input ends here, invalid!
    
    size_t position = 0;
    // While not triple single quote and not at end of input, take.
    while(
        input[position] != '\0' &&
        input[position + 1 ] != '\0' &&
        input[position + 2 ] != '\0' &&
        !(
            input[position] == quote &&
            input[position + 1 ] == quote &&
            input[position + 2 ] == quote
        )
    ) {
        position ++;
    }
    // Don't include the newline in the parsed value.
    if(input[position-1] == '\n') position--;

    // Allocate space for return
    char* result = malloc((position + 1) * sizeof(char));
    strncpy(result, input, position);
    result[position] = '\0'; // Terminator

    // If we did drop a newline, fix the pointer.
    if(input[position] == '\n') position++;

    // Drop:
    // - Starting heredoc terminator and any trailing whitespace + first newline
    // - Ending heredoc terminator (3 single quotes)
    (*inputPointer) += (input - originalInput) + position + 3;
    (*interpolate) = quote == '"'; // Flag double for interpolation
    return result;
}

char* replace_escapes(char* input, size_t inputLength) {
    char* output = malloc((inputLength + 1) * sizeof(char));
    size_t j = 0; // output position
    for(size_t i = 0; i < inputLength; i++) {
        // Not an escape
        if(input[i] != '\\') {
            output[j++] = input[i];
            continue;
        }
        i++;
        if(input[i] == 'n') { output[j++] = '\n'; continue; }
        if(input[i] == 'r') { output[j++] = '\r'; continue; }
        if(input[i] == 't') { output[j++] = '\t'; continue; }
        if(input[i] == 'f') { output[j++] = '\f'; continue; }
        if(input[i] == 'b') { output[j++] = '\b'; continue; }
        if(input[i] == 'u') {
            char a = input[++i];
            char b = input[++i];
            char c = input[++i];
            char d = input[++i];
            bool valid = true;

            // Convert
            if(a >= 'a' && a <= 'f') a = toupper(a);
            if(b >= 'a' && b <= 'f') b = toupper(b);
            if(c >= 'a' && c <= 'f') c = toupper(c);
            if(d >= 'a' && d <= 'f') d = toupper(d);

            // Validate
            if(!((a >= '0' && a <= '9') || (a >= 'A' && a <= 'F'))) valid = false;
            if(!((b >= '0' && b <= '9') || (b >= 'A' && b <= 'F'))) valid = false;
            if(!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) valid = false;
            if(!((d >= '0' && d <= '9') || (d >= 'A' && d <= 'F'))) valid = false;
            if(valid == false) {
                i -= 4;
                output[j++] = 'u';
                continue;
            }

            int nA = a >= 'A' ? a - 'A' + 10 : a - '0';
            int nB = b >= 'A' ? b - 'A' + 10 : b - '0';
            int nC = c >= 'A' ? c - 'A' + 10 : c - '0';
            int nD = d >= 'A' ? d - 'A' + 10 : d - '0';

            int binary = (nA << 12) | (nB << 8) | (nC << 4) | nD;

            int charsNeeded = 1;
            if(binary > 0x0000007F) charsNeeded++;
            if(binary > 0x000007FF) charsNeeded++;

            if(charsNeeded == 1) {
                char out = (binary & 0b0000000001111111);
                output[j++] = out;
                continue;
            }
            if(charsNeeded == 2) {
                int bin1 = (binary & 0b0000011111000000) >> 6;
                int bin2 = (binary & 0b0000000000111111);
                char out1 = 0b11000000 | bin1;
                char out2 = 0b10000000 | bin2;
                output[j++] = out1;
                output[j++] = out2;
                continue;
            }
            if(charsNeeded == 3) {
                int bin1 = (binary & 0b1111000000000000) >> 12;
                int bin2 = (binary & 0b0000111111000000) >> 6;
                int bin3 = (binary & 0b0000000000111111);
                char out1 = 0b11100000 | bin1;
                char out2 = 0b10000000 | bin2;
                char out3 = 0b10000000 | bin3;
                output[j++] = out1;
                output[j++] = out2;
                output[j++] = out3;
                continue;
            }
            continue;
        }
        output[j++] = input[i];
    }
    output[j] = '\0';

    // Minify! Since each escape removes at least one character,
    // The output is now shorter than the original input!
    output = realloc(output, j + 1);
    return output;
}

char* replace_substitutions(char* input, size_t inputLength) {
    size_t outputSize = (inputLength + 1) * sizeof(char);
    char* output = malloc(outputSize);
    char* varName = malloc(257 * sizeof(char));
    size_t j = 0; // output position
    size_t k; // varname position
    char* varValue;
    size_t varValueLength;
    char* tmp;
    for(size_t i = 0; i < inputLength; i++) {
        if(input[i] == '$' && i+1 <= inputLength && input[i+1] == '{') {
            i++;
            // Read varName
            k = 0;
            while(++i < inputLength && input[i] != '}' && k < 256) {
                varName[k++] = input[i];
            }
            varName[k] = '\0';
            varValue = getenv(varName);
            if(varValue == NULL) {
                // No such env var found. Will leave uninterpolated.
                TTY_E(TTY_PURPLE, "WARN: Interpolated var not found; leaving as-is.\n");
                output[j++] = '$';
                output[j++] = '{';
                tmp = output + j;
                strcpy(tmp, varName);
                j += k;
                tmp = NULL;
                output[j++] = '}';
                continue;
            }
            varValueLength = strlen(varValue);
            // Resize output to account for new data.
            outputSize = outputSize + (varValueLength * sizeof(char));
            tmp = realloc(output, outputSize);
            if(tmp != NULL) output = tmp;
            else {
                TTY_E(TTY_RED, "ERROR: Ran out of memory during interpolation of ${%s}. Aborting.", varName);
                exit(1);
            }
            tmp =  output + j;
            strcpy(tmp, varValue);
            j += varValueLength;
            tmp = NULL;
            continue;
        }
        output[j++] = input[i];
    }
    output[j] = '\0';

    // Minify! Since each escape removes at least one character,
    // The output is now shorter than the original input!
    output = realloc(output, j + 1);
    free(varName);
    return output;
}

char* parse_value(char** inputPointer, size_t inputLength) {
    char* input = *inputPointer;
    // Check if input is NULL or inputLength is 0
    if (input == NULL || inputLength == 0)
        return NULL;
    char* result = NULL;
    bool interpolate = true;

    // IMPORTANT: Order matters here. Longer magic before shorter.
    if(result == NULL) result = parse_heredoc_value(&input, inputLength, &interpolate);
    if(result == NULL) result = parse_quoted_value(&input, inputLength, &interpolate);
    if(result == NULL) result = parse_bare_value(&input, inputLength);
    if(result == NULL) return NULL;

    // Update our input pointer to wherever the above parsers left off.
    (*inputPointer) = input;

    // Process Escapes
    char* unescaped = replace_escapes(result, strlen(result));
    free(result); // Won't be using this from here.
    result = NULL; // Take care of this dangling pointer.

    // Process Interpolations
    char* interpolated;
    if(interpolate) {
        interpolated = replace_substitutions(unescaped, strlen(unescaped));
        free(unescaped);
    } else {
        interpolated = unescaped;
        // Do not free it here, as doing so will also free interpolated, since
        // they both point to the same memory location.
    }
    unescaped = NULL; // No!

    return interpolated;
}

void test_parse_bare_value() {
    describeSection("char* parse_bare_value(char** inputPointer, size_t inputLength);");
    describe("bare value");
    char* case1 = "bar\n";
    char* rslt1 = parse_value(&case1, strlen(case1));
    assert_str_equal(case1, "\n");
    assert_str_equal(rslt1, "bar");
    free(rslt1);

    describe("bare value with spaces");
    char* case2 = "bar baz boop \n";
    char* rslt2 = parse_value(&case2, strlen(case2));
    assert_str_equal(case2, " \n");
    assert_str_equal(rslt2, "bar baz boop");
    free(rslt2);

    describe("bare value with comment");
    char* case3 = "bar baz boop # I am a comment\n";
    char* rslt3 = parse_value(&case3, strlen(case3));
    assert_str_equal(case3, " # I am a comment\n");
    assert_str_equal(rslt3, "bar baz boop");
    free(rslt3);
}

void test_parse_quoted_value_single() {
    describeSection("char* parse_quoted_value(char** inputPointer, size_t inputLength, bool* interpolate); // SINGLE QUOTE");
    describe("simple value");
    char* case1 = "'bar'\n";
    char* rslt1 = parse_value(&case1, strlen(case1));
    assert_str_equal(case1, "\n");
    assert_str_equal(rslt1, "bar");
    free(rslt1);

    describe("value with spaces");
    char* case2 = "'bar baz boop' \n";
    char* rslt2 = parse_value(&case2, strlen(case2));
    assert_str_equal(case2, " \n");
    assert_str_equal(rslt2, "bar baz boop");
    free(rslt2);

    describe("value with comment");
    char* case3 = "'bar baz boop' # I am a comment\n";
    char* rslt3 = parse_value(&case3, strlen(case3));
    assert_str_equal(case3, " # I am a comment\n");
    assert_str_equal(rslt3, "bar baz boop");
    free(rslt3);

    describe("value with escaped quote");
    char* case4 = "'bar baz\\' boop' # I am a comment\n";
    char* rslt4 = parse_value(&case4, strlen(case4));
    assert_str_equal(case4, " # I am a comment\n");
    assert_str_equal(rslt4, "bar baz' boop");
    free(rslt4);
}

void test_parse_quoted_value_double() {
    describeSection("char* parse_quoted_value(char** inputPointer, size_t inputLength, bool* interpolate); // DOUBLE QUOTE");
    describe("simple value");
    char* case1 = "\"bar\"\n";
    char* rslt1 = parse_value(&case1, strlen(case1));
    assert_str_equal(case1, "\n");
    assert_str_equal(rslt1, "bar");
    free(rslt1);

    describe("value with spaces");
    char* case2 = "\"bar baz boop\" \n";
    char* rslt2 = parse_value(&case2, strlen(case2));
    assert_str_equal(case2, " \n");
    assert_str_equal(rslt2, "bar baz boop");
    free(rslt2);

    describe("value with comment");
    char* case3 = "\"bar baz boop\" # I am a comment\n";
    char* rslt3 = parse_value(&case3, strlen(case3));
    assert_str_equal(case3, " # I am a comment\n");
    assert_str_equal(rslt3, "bar baz boop");
    free(rslt3);

    describe("value with escaped quote");
    char* case4 = "\"bar baz\\\" boop\" # I am a comment\n";
    char* rslt4 = parse_value(&case4, strlen(case4));
    assert_str_equal(case4, " # I am a comment\n");
    assert_str_equal(rslt4, "bar baz\" boop");
    free(rslt4);
}

void test_parse_heredoc_value_single() {
    describeSection("char* parse_heredoc_value(char** inputPointer, size_t inputLength, bool* interpolate); // SINGLE QUOTE");
    describe("simple heredoc value");
    char* case1 = "'''\nbar\n'''\n";
    char* rslt1 = parse_value(&case1, strlen(case1));
    assert_str_equal(case1, "\n");
    assert_str_equal(rslt1, "bar");
    free(rslt1);

    describe("heredoc value with trailing spaces");
    char* case2 = "'''     \nbar baz boop\n''' \n";
    char* rslt2 = parse_value(&case2, strlen(case2));
    assert_str_equal(case2, " \n");
    assert_str_equal(rslt2, "bar baz boop");
    free(rslt2);

    describe("accept heredoc value with badly formatted start");
    char* case3 = "'''bar baz boop\n''' # I am a comment\n";
    char* rslt3 = parse_value(&case3, strlen(case3));
    assert_str_equal(case3, " # I am a comment\n");
    assert_str_equal(rslt3, "bar baz boop");
    free(rslt3);

    describe("accept heredoc value with badly formatted end");
    char* case4 = "'''\nbar baz boop''' # I am a comment\n";
    char* rslt4 = parse_value(&case4, strlen(case4));
    assert_str_equal(case4, " # I am a comment\n");
    assert_str_equal(rslt4, "bar baz boop");
    free(rslt4);
}

void test_parse_heredoc_value_double() {
    describeSection("char* parse_heredoc_value(char** inputPointer, size_t inputLength, bool* interpolate); // DOUBLE QUOTE");
    describe("simple heredoc value");
    char* case1 = "\"\"\"\nbar\n\"\"\"\n";
    char* rslt1 = parse_value(&case1, strlen(case1));
    assert_str_equal(case1, "\n");
    assert_str_equal(rslt1, "bar");
    free(rslt1);

    describe("heredoc value with trailing spaces");
    char* case2 = "\"\"\"     \nbar baz boop\n\"\"\" \n";
    char* rslt2 = parse_value(&case2, strlen(case2));
    assert_str_equal(case2, " \n");
    assert_str_equal(rslt2, "bar baz boop");
    free(rslt2);

    describe("accept heredoc value with badly formatted start");
    char* case3 = "\"\"\"bar baz boop\n\"\"\" # I am a comment\n";
    char* rslt3 = parse_value(&case3, strlen(case3));
    assert_str_equal(case3, " # I am a comment\n");
    assert_str_equal(rslt3, "bar baz boop");
    free(rslt3);

    describe("accept heredoc value with badly formatted end");
    char* case4 = "\"\"\"\nbar baz boop\"\"\" # I am a comment\n";
    char* rslt4 = parse_value(&case4, strlen(case4));
    assert_str_equal(case4, " # I am a comment\n");
    assert_str_equal(rslt4, "bar baz boop");
    free(rslt4);
}

void test_replace_escapes() {
    describeSection("char* replace_escapes(char* input, size_t inputLength);");
    char* test;
    char* result;

    describe("linefeed");
    test = "value\\n\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\n");
    free(result);

    describe("carriage return");
    test = "value\\r\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\r");
    free(result);

    describe("tab");
    test = "value\\t\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\t");
    free(result);

    describe("form feed");
    test = "value\\f\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\f");
    free(result);

    describe("backspace");
    test = "value\\b\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\b");
    free(result);

    describe("double quote");
    test = "value\\\"\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\"");
    free(result);

    describe("single quote");
    test = "value\\'\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value'");
    free(result);

    describe("backslash");
    test = "value\\\\\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\\");
    free(result);

    describe("unicode - UTF-8 sequence size 1");
    test = "value\\u0024\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value$");
    free(result);

    describe("unicode - UTF-8 sequence size 2");
    test = "value\\u05e9\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\u05e9");
    free(result);

    describe("unicode - UTF-8 sequence size 3");
    test = "value\\uEFab\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "value\uefAB");
    free(result);
}

void test_replace_substitutions() {
    describeSection("char* replace_substitutions(char* input, size_t inputLength);");
    char* test;
    char* result;

    describe("Existing envvar");
    test = "FooBarBaz${INTERPOLATION_TEST_VAR}Bux\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "FooBarBazInterpolatedBux");
    free(result);

    describe("Nonexistant envvar");
    test = "FooBarBaz${INTERPOLATION_FAKE_VAR}Bux\n";
    result = parse_value(&test, strlen(test));
    assert_str_equal(test, "\n");
    assert_str_equal(result, "FooBarBaz${INTERPOLATION_FAKE_VAR}Bux");
    free(result);

}


void test_value() {
    describeFile("value.c");
    test_parse_bare_value();
    test_parse_quoted_value_single();
    test_parse_quoted_value_double();
    test_parse_heredoc_value_single();
    test_parse_heredoc_value_double();
    test_replace_escapes();
    test_replace_substitutions();
}