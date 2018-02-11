#ifndef __MAIN_H_
#define __MAIN_H_

#include "mytypes.h"

void init(char* space, short dataSize);
void dispatch(void);
void processLine(char* line, token* t);

#endif

