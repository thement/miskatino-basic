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

char extraFuncArgCnt[] = {1, 1, 1};

static char* commonStrings = CONST_COMMON_STRINGS;
static char* parsingErrors = CONST_PARSING_ERRORS;

#define VARS_SPACE_SIZE 512
#define PROG_SPACE_SIZE 4096
#define LINE_SIZE 80

char dataSpace[VARS_SPACE_SIZE + PROG_SPACE_SIZE];
char lineSpace[LINE_SIZE * 3];

static char storageOpened = 0;

void sysPutc(char c) {
    char s[32];
    sprintf(s, "jsPutc(%d)", c);
    emscripten_run_script(s);
}

void sysEcho(char c) {
    sysPutc(c);
}

void sysPoke(unsigned long addr, uchar value) {
    dataSpace[addr] = value;
}

uchar sysPeek(unsigned long addr) {
    return dataSpace[addr];
}

numeric sysMillis(numeric div) {
    if (div <= 1) {
        return emscripten_run_script_int("timeMs()");
    } else {
        char s[32];
        sprintf(s, "timeMs(%d)", div);
        return emscripten_run_script_int(s);
    }
}

void pinOut(int pin, int value) {
    char s[32];
    sprintf(s, "pinOut(%d,%d)", pin, value);
    emscripten_run_script(s);
}

int pinIn(int pin, int analog) {
    char s[32];
    sprintf(s, "pinIn(%d,%d)", pin, analog);
    return emscripten_run_script_int(s);
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
        case 0x019C: // PIN
            return 1;
        case 0x01CF: // ADC
            return 2;
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
            pinOut(args[0], args[1]);
            break;
    }
}

numeric extraFunction(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            return sysPeek(args[0]);
        case 1:
            return pinIn(args[0], 0);
        case 2:
            return pinIn(args[0], 1);
    }
    return 0;
}

char storage(int arg) {
    char s[32];
    sprintf(s, "storageOp(%d)", arg);
    return emscripten_run_script_int(s);
}

char storageOperation(void* data, short size) {
    int i;
    if (data == NULL) {
        if (storageOpened) {
            storage(-'C');
            storageOpened = 0;
        }
        if (size != 0) {
            storageOpened = 1;
            return storage(size > 0 ? -'W' : -'R');
        }
        return 1;
    }
    if (size > 0) {
        for (i = 0; i < size; i++) {
            storage(((unsigned char*) data)[i]);
        }
    } else {
        for (i = 0; i < -size; i++) {
            ((unsigned char*) data)[i] = storage(-'G');
        }
    }
    return 1;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
void initBasic() {
    init(VARS_SPACE_SIZE, LINE_SIZE, PROG_SPACE_SIZE);
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

