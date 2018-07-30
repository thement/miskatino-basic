#include "mystdlib.h"
#include "editor.h"
#include "utils.h"
#include "tokens.h"
#include "extern.h"

extern token* toksBody;
char* prgStore;
short prgSize;
static char lineSpacePos;

void resetEditor(void) {
    ((prgline*)prgStore)->num = 0;
    prgSize = 2;
    lineSpacePos = 0;
}

void initEditor(char* prgBody) {
    prgStore = prgBody;
    resetEditor();
}

char readLine() {
    if (lastInput == '\r' || lastInput == '\n') {
        trim(lineSpace);
        lineSpace[lineSpacePos] = 0;
        lineSpacePos = 0;
        sysEcho('\n');
        return 1;
    } else if (lastInput == '\b' || lastInput == 127) {
        if (lineSpacePos > 0) {
            lastInput = '\b';
            lineSpacePos -= 1;
        }
    } else if (lastInput >= ' ') {
        lineSpace[lineSpacePos++] = lastInput;
    }
    sysEcho(lastInput);
    return 0;
}

short lineSize(prgline* p) {
    return p->str.len + 3;
}

prgline* nextLine(prgline* p) {
    return (prgline*)(void*)((char*)(void*)p + lineSize(p));
}

prgline* findLine(short num) {
    prgline* p = (prgline*)(void*)prgStore;
    while (p->num != 0 && p->num < num) {
        p = nextLine(p);
    }
    return p;
}

void injectLine(char* s, short num) {
    unsigned char len;
    prgline* p = findLine(num);
    if (p->num == num) {
        len = (char*)(void*)nextLine(p) - (char*)(void*)p;
        memmove(p, nextLine(p), prgStore + prgSize - (char*)(void*)nextLine(p));
        prgSize -= len;
    }
    len = strlen(s);
    if (len > 0) {
        memmove((char*)(void*)p + len + 3, p, prgStore + prgSize - (char*)(void*)p);
        prgSize += len + 3;
        p->num = num;
        p->str.len = len;
        memcpy(p->str.text, s, len);
    }
}

char editorSave(void) {
    if (!storageOperation(NULL, 1)) {
        return 0;
    }
    storageOperation(&prgSize, sizeof(prgSize));
    storageOperation(prgStore, prgSize);
    storageOperation(NULL, 0);
    return 1;
}

char editorLoad(void) {
    if (!storageOperation(NULL, -1)) {
        return 0;
    }
    storageOperation(&prgSize, (short) -sizeof(prgSize));
    storageOperation(prgStore, -prgSize);
    storageOperation(NULL, 0);
    return 1;
}

char editorLoadParsed() {
    void* p = prgStore;
    unsigned char len;
    if (!storageOperation(NULL, -1)) {
        return 0;
    }
    storageOperation(lineSpace, -2);
    while (1) {
        storageOperation(p, (short) -sizeof(short));
        if (*((short*)p) == 0) {
            break;
        }
        parseLine(lineSpace, toksBody);
        p = (char*)p + sizeof(short);
        storageOperation(&len, (short) -sizeof(len));
        storageOperation(lineSpace, -len);
        lineSpace[len] = 0;
        parseLine(lineSpace, toksBody);
        len = tokenChainSize(toksBody);
        *((char*)p) = len;
        memcpy((char*)p + 1, toksBody, len);
        p = (char*)p + len + 1;
    }
    storageOperation(NULL, 0);
    prgSize = ((char*)p - (char*)(void*)prgStore) + sizeof(short);
    return 1;
}

