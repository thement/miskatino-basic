#include "mystdlib.h"
#include "tokens.h"
#include "editor.h"
#include "exectoks.h"
#include "utils.h"
#include "extern.h"
#include "textual.h"

static short listLine, listPage;
static token* toksBody;
char mainState;

#if 0
void printToken(token* t) {
    switch (t->type) {
        case TT_NUMBER:
            outputStr("INT=");
            outputInt(t->body.integer);
            break;
        case TT_NAME:
            outputStr("NAME=");
            outputNStr(&(t->body.str));
            break;
        case TT_VARIABLE:
            outputStr("VAR=");
            outputNStr(&(t->body.str));
            break;
        case TT_FUNCTION:
            outputStr("FN=");
            outputNStr(&(t->body.str));
            break;
        case TT_COMMAND:
            outputStr("CMD=");
            outputInt(t->body.command);
            break;
        case TT_LITERAL:
            outputStr("STR=\"");
            outputNStr(&(t->body.str));
            outputStr("\"");
            break;
        case TT_COMMENT:
            outputStr("REM=\"");
            outputNStr(&(t->body.str));
            outputStr("\"");
            break;
        case TT_SYMBOL:
            outputStr("SYM=");
            outputChar(t->body.symbol);
            break;
        case TT_ARRAY:
            outputStr("ARR=");
            outputChar(t->body.symbol);
            break;
        case TT_FUNC_END:
            outputStr("FE=%d");
            outputInt(t->body.symbol);
            break;
        case TT_NONE:
            outputChar('N');
            break;
        case TT_SEPARATOR:
            outputChar(';');
            break;
        default:
            outputChar('E');
            break;
    }
}

void printTokens(token* t) {
    while (1) {
        printToken(t);
        outputChar(' ');
        if (tokenClass(t) == TT_NONE) {
            break;
        }
        t = nextToken(t);
    }
    outputCr();
}
#else
void printTokens(token* t) {
}
#endif

void printProgram(void) {
    prgline* p = findLine(listLine);
    if (p->num == 0 && listLine > 1) {
        p = findLine(1);
    }
    short lineCount = 0;
    while (p->num != 0 && lineCount < listPage) {
        listLine = p->num + 1;
        outputInt(p->num);
        outputChar(' ');
        outputNStr(&(p->str));
        outputCr();
        p = findLine(p->num + 1);
        lineCount += 1;
    }
}

void listProgram(token* t) {
    t = nextToken(nextToken(t));
    if (t->type == TT_NUMBER) {
        listLine = t->body.integer;
        t = nextToken(t);
        if (t->type == TT_NUMBER) {
            listPage = t->body.integer;
        }
    }
    printProgram();
}

void executeSteps(char* lineBody, token* tokensBody) {
    token* t = nextToken(nextToken(tokensBody));
    mainState |= STATE_STEPS;
    executeNonParsed(lineBody, tokensBody, t->type == TT_NUMBER ? t->body.integer : 1);
}

void executeRun(char* lineBody, token* tokensBody) {
    nextLineNum = 1;
    if (editorSave()) {
        editorLoadParsed(lineBody, tokensBody);
        executeParsedRun();
        editorLoad();
    } else {
        executeNonParsed(lineBody, tokensBody, -1);
    }
}

void manualSave(void) {
    editorSave();
    outputConstStr(ID_COMMON_STRINGS, 6, NULL); // Saved
    outputChar(' ');
    outputInt(prgSize + 2);
    outputChar(' ');
    outputConstStr(ID_COMMON_STRINGS, 8, NULL); // bytes
    outputCr();
}

void manualLoad(void) {
    if (editorLoad()) {
        outputConstStr(ID_COMMON_STRINGS, 7, NULL); // Loaded
        outputChar(' ');
        outputInt(prgSize + 2);
        outputChar(' ');
        outputConstStr(ID_COMMON_STRINGS, 8, NULL); // bytes
        outputCr();
    } else {
        outputConstStr(ID_COMMON_STRINGS, 9, NULL); // bytes
        outputCr();
    }
}

void prgReset(void) {
    resetEditor();
    resetTokenExecutor();
}

void showInfo(void) {
    outputConstStr(ID_COMMON_STRINGS, 1, NULL); // code:
    outputInt(prgSize);
    outputCr();
    outputConstStr(ID_COMMON_STRINGS, 2, NULL); // vars:
    outputInt(varSize());
    outputCr();
    outputConstStr(ID_COMMON_STRINGS, 3, NULL); // next:
    outputInt(nextLineNum);
    outputCr();
}

void metaOrError(token* t, char* line) {
    numeric h = tokenHash(t);
    if (h == 0x31A) { // QUIT
        sysQuit();
    } else if (h == 0x3B6) { // LIST
        listProgram(t);
    } else if (h == 0x312) { // STEP
        executeSteps(line, t);
    } else if (h == 0x1AC) { // RUN
        executeRun(line, t);
    } else if (h == 0x375) { // SAVE
        manualSave();
    } else if (h == 0x39A) { // LOAD
        manualLoad();
    } else if (h == 0x69A) { // RESET
        prgReset();
    } else if (h == 0x3B3) { // INFO
        showInfo();
    } else {
        getParseErrorMsg(line);
        outputStr(line);
        outputChar(' ');
        outputInt((long)(getParseErrorPos() - line) + 1);
        outputCr();
    }
}

void processLine() {
    if (lineSpace[0] == 0) {
        return;
    }
    parseLine(lineSpace, toksBody);
    printTokens(toksBody);
    if (getParseErrorPos() != NULL) {
        metaOrError(toksBody, lineSpace);
        return;
    }
    if (toksBody->type != TT_NUMBER) {
        executeTokens(toksBody);
    } else {
        injectLine(skipSpaces(skipDigits(lineSpace)), toksBody->body.integer);
    }
}

void preload(char* line, token* t) {
    if (editorLoadParsed(line, t)) {
        outputConstStr(ID_COMMON_STRINGS, 10, NULL); // code found, autorun message
        outputCr();
        //sysDelay(1000);
        //if (sysGetc() < 0) {
        //    executeParsedRun();
        //} else {
            outputConstStr(ID_COMMON_STRINGS, 11, NULL); // canceled
            outputCr();
        //}
    }
    prgReset();
}

void init(short dataSize, short lineSize) {
    outputCr();
    outputConstStr(ID_COMMON_STRINGS, 0, NULL); // Miskatino vX.X
    outputCr();
    initEditor(dataSpace + dataSize);
    initTokenExecutor(dataSpace, dataSize);
    listLine = 1;
    listPage = 3;
    mainState = STATE_INTERACTIVE;
    toksBody = (token*)(void*) (lineSpace + lineSize);
    preload(lineSpace, toksBody);
}

void dispatch() {
    if (lastInput == 3) {
        mainState |= STATE_BREAK;
    }
    if ((mainState & (STATE_RUN | STATE_SLOWED)) == STATE_RUN) {
        return;
    }
    switch (mainState & STATE_SLOWED) {
        case STATE_DELAY:
            dispatchDelay();
            return;
        case STATE_INPUT:
            return;
        case STATE_BREAK:
            dispatchBreak();
            return;
    }
    if ((mainState & STATE_STEPS) != 0) {
        executeNonParsed(lineSpace, toksBody, 0);
    } else {
        if (lastInput >= 0) {
            if (readLine()) {
                processLine();
            }
        }
    }
    return;
}

