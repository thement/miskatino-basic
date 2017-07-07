#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "tokens.h"
#include "expr.h"
#include "utils.h"

char* cmds[] = {
    "REM",
    "PRINT",
    "INPUT",
    "IF",
    "GOTO",
    "GOSUB",
    "RETURN",
    "END",
    ""
};

char* errorMsgs[] = {
    "ok",
    "command or variable expected",
    "'=' expected",
    "variable name expected",
    "';' expected",
    "extra characters at line end",
    "unexpected error",
    "linenum out of range",
    "line number expected",
    "unexpected symbol",
    
};

char* cur;
token* curTok;
token* prevTok;
char* parseError;
char parseErrorCode;

char* getCurTokPos() {
    return cur;
}

int tokenSize(token* t) {
    switch (t->type) {
        case TT_NUMBER:
            return 1 + sizeof(t->body.integer);
        case TT_NAME:
        case TT_COMMENT:
        case TT_LITERAL:
        case TT_FUNCTION:
        case TT_VARIABLE:
            return 2 + t->body.str.len;
        case TT_FUNC_END:
        case TT_SYMBOL:
        case TT_COMMAND:
            return 1 + sizeof(t->body.symbol);
        case TT_NONE:
        case TT_ERROR:
        case TT_SEPARATOR:
            return 1;
    }
    return 0;
}

void copyToken(void* dst, void* src) {
    memcpy(dst, src, tokenSize(src));
}

void setTokenError(char* pos, char code) {
    parseErrorCode = code;
    parseError = pos;
    curTok->type = TT_ERROR;
}

token* nextToken(void* t) {
    return t + tokenSize(t);
}

void skipTokenInInput(int skip) {
    cur = skipSpaces(cur + skip);
}

void advance(char* s) {
    cur = skipSpaces(s);
    prevTok = curTok;
    curTok = nextToken(curTok);
}

void trySubstCmd(void) {
    short i = 0;
    for (i = 0; cmds[i][0] != 0; i++) {
        if (tokenNameEqual(curTok, cmds[i])) {
            curTok->type = TT_COMMAND;
            curTok->body.command = i;
            nextToken(curTok)->type = TT_ERROR;
        }
    }
}

int parseName(char checkCmd) {
    short i = 0;
    if (!isalpha(*cur)) {
        return 0;
    }
    curTok->type = TT_NAME;
    while (isalnum(cur[i])) {
        curTok->body.str.text[i] = toupper(cur[i]);
        i++;
    }
    curTok->body.str.len = i;
    if (checkCmd) {
        trySubstCmd();
    }
    advance(cur + i);
    return 1;
}

int parseNumber(void) {
    if (!isdigit(*cur)) {
        return 0;
    }
    curTok->type = TT_NUMBER;
    curTok->body.integer = 0;
    while (isdigit(*cur)) {
        curTok->body.integer = curTok->body.integer * 10 + (*cur) - '0';
        cur++;
    }
    advance(cur);
    return 1;
}

int parseNone(void) {
    if (*cur != 0) {
        setTokenError(cur, 5);
        return 0;
    }
    curTok->type = TT_NONE;
    return 1;
}

int parseComment(void) {
    unsigned char len = strlen(cur);
    curTok->type = TT_COMMENT;
    curTok->body.str.len = len;
    memcpy(&(curTok->body.str.text), cur, len);
    advance(cur + len);
    return parseNone();
}

int parseLiteral() {
    if (*cur != '"') {
        return 0;
    }
    int i = 1;
    curTok->type = TT_LITERAL;
    while (cur[i] != 0 && cur[i] != '"') {
        curTok->body.str.text[i - 1] = cur[i];
        i++;
    }
    curTok->body.str.len = i - 1;
    advance(cur + i + (cur[i] == '"' ? 1 : 0));
    return 1;
}

void parseSymbol() {
    curTok->type = TT_SYMBOL;
    char c = 0;
    if (cur[0] == '<') {
        if (cur[1] == '>') {
            c = '#';
        } else if (cur[1] == '=') {
            c = '{';
        }
    } else if (cur[0] == '>' && cur[1] == '=') {
        c = '}';
    }
    if (c != 0) {
        cur++;
    } else {
        c = cur[0];
    }
    curTok->body.symbol = c;
    advance(cur + 1);
}

int parseLineNumber(void) {
    char* start = cur;
    if (!parseNumber()) {
        return 1;
    }
    if (prevTok->body.integer < 1 || prevTok->body.integer > MAX_LINE_NUMBER) {
        setTokenError(start, 7);
        return 0;
    }
    return 1;
}

int parseAssignment(void) {
    if (*cur != '=') {
        setTokenError(cur, 2);
        return 0;
    }
    parseSymbol();
    return parseExpression() && parseNone();
}

int parseExprOrLiteral(void) {
    if (parseLiteral()) {
        return 1;
    }
    return parseExpression();
}

int parseSemicolon(void) {
    if (*cur != ';') {
        setTokenError(cur, 4);
        return 0;
    }
    curTok->type = TT_SEPARATOR;
    advance(cur + 1);
    return 1;
}

int parseVar(void) {
    if (!parseName(0)) {
        setTokenError(cur, 3);
        return 0;
    }
    prevTok->type = TT_VARIABLE;
    return 1;
}

int parseVarList(void) {
    if (!parseVar()) {
        return 0;
    }
    while (*cur != 0) {
        if (!parseSemicolon() || !parseVar()) {
            return 0;
        }
    }
    return parseNone();
}

int parsePrintList(void) {
    if (!parseExprOrLiteral()) {
        return 0;
    }
    while (*cur != 0) {
        if (!parseSemicolon() || !parseExprOrLiteral()) {
            return 0;
        }
    }
    return parseNone();
}

int parseLabel(void) {
    if (parseNumber()) {
        return parseNone();
    }
    setTokenError(cur, 8);
    return 0;
}

int parseStatement(void);

int parseConditional(void) {
    if (!parseExpression()) {
        return 0;
    }
    return parseSemicolon() && parseStatement();
}

void parseSpecialWithError() {
    curTok = nextToken(curTok);
    while (parseName(0) || parseNumber() || parseLiteral()) {
    }
    curTok->type = TT_ERROR;
}

int parseStatement(void) {
    char cmd;
    if (!parseName(1)) {
        setTokenError(cur, 1);
    } else if (prevTok->type != TT_COMMAND) {
        if (parseAssignment()) {
            return 1;
        } else {
            parseSpecialWithError();
            return 0;
        }
    }
    cmd = prevTok->body.command;
    if (cmd == CMD_REM) {
        return parseComment();
    } else if (cmd == CMD_GOTO || cmd == CMD_GOSUB) {
        return parseLabel();
    } else if (cmd == CMD_RETURN || cmd == CMD_END) {
        return parseNone();
    } else if (cmd == CMD_PRINT) {
        return parsePrintList();
    } else if (cmd == CMD_INPUT) {
        return parseVarList();
    } else if (cmd == CMD_IF) {
        return parseConditional();
    }
    setTokenError(cur, 6);
    return 0;
}

void parseLine(char* line, void* tokens) {
    cur = line;
    curTok = tokens;
    prevTok = NULL;
    setTokenError(NULL, 0);
    if (parseLineNumber()) {
        parseStatement();
    }
}

int tokenClass(token* t) {
    return t->type & 0xF0;
}

int tokenNameEqual(token* t, char* s) {
    if (tokenClass(t) != TT_NAME) {
        return 0;
    }
    if (t->body.str.len != strlen(s)) {
        return 0;
    }
    return memcmp(t->body.str.text, s, t->body.str.len) == 0;
}

char* getParseErrorPos(void) {
    return parseError;
}

char* getParseErrorMsg(void) {
    return errorMsgs[parseErrorCode];
}

