#ifndef __EDITOR_H_
#define __EDITOR_H_

#include "utils.h"

#define MAX_PRG_SIZE 4096

typedef struct prgline {
    short num;
    nstring str;
} __attribute__((packed)) prgline;

void initEditor(void);
prgline* findLine(int num);
void injectLine(char* s, int num);

#endif

