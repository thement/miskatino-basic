#include <emscripten.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../core/main.h"
#include "../core/utils.h"
#include "../core/textual.h"
#include "../core/tokens.h"
#include "../core/extern.h"

char extraCmdArgCnt[] = {2, 2};

char extraFuncArgCnt[] = {1, 2};

static char* commonStrings = CONST_COMMON_STRINGS;
static char* parsingErrors = CONST_PARSING_ERRORS;

#define VARS_SPACE_SIZE 512
#define PROG_SPACE_SIZE 4096
#define LINE_SIZE 80

char dataSpace[VARS_SPACE_SIZE + PROG_SPACE_SIZE];
char lineSpace[LINE_SIZE * 3];

volatile char interrupted;

void sysPutc(char c) {
    char s[64];
    sprintf(s, "jsPutc(%d)", c);
    emscripten_run_script(s);
}

void sysEcho(char c) {
    sysPutc(c);
}

void sysQuit(void) {
}

void sysPoke(unsigned long addr, uchar value) {
    dataSpace[addr] = value;
}

uchar sysPeek(unsigned long addr) {
    return dataSpace[addr];
}

numeric sysMillis(void) {
    return emscripten_run_script_int("millis()");
}

void outputConstStr(char strId, char index, char* w) {
    char* s;
    switch (strId) {
        case ID_COMMON_STRINGS:
            s = commonStrings;
            break;
        case ID_PARSING_ERRORS:
            s = parsingErrors;
            break;
        default:
            return;
    }
    while (index > 0) {
        while (*s++ != '\n') {
        }
        index -= 1;
    }
    while (*s != '\n') {
        if (w) {
            *(w++) = (*s++);
        } else {
            sysPutc(*s++);
        }
    }
    if (w) {
        *w = 0;
    }
}

static numeric power(numeric base, numeric exp) {
    return exp < 1 ? 1 : base * power(base, exp - 1);
}

short extraCommandByHash(numeric h) {
    switch (h) {
        case 0x036F: // POKE
            return CMD_EXTRA + 0;
        case 0x019C: // PIN - just prints argument values for test
            return CMD_EXTRA + 1;
        default:
            return -1;
    }
}

short extraFunctionByHash(numeric h) {
    switch (h) {
        case 0x0355: // PEEK
            return 0;
        case 0x06FC: // POWER - for test purpose
            return 1;
        default:
            return -1;
    }
}

void extraCommand(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            sysPoke(args[0], args[1]);
            break;
        case 1:
            break;
    }
}

numeric extraFunction(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            return sysPeek(args[0]);
        case 1:
            return power(args[1], args[0]);
    }
    return 0;
}

char storageOperation(void* data, short size) {
    return 1;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
void initBasic() {
    init(VARS_SPACE_SIZE, LINE_SIZE);
}

EMSCRIPTEN_KEEPALIVE
void processBasic() {
    dispatch();
}

EMSCRIPTEN_KEEPALIVE
void inputBasic(int c) {
    lastInput = (char) (c & 0xFF);
}

}

