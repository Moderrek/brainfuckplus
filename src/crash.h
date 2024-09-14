#ifndef CRASH_H_INCLUDED
#define CRASH_H_INCLUDED

#include "state.h"

void crash(BFData* data, const char* message, int code);
void crash_file(BFData* data, const char* message, int code, const char* filename, int pos);

#endif // CRASH_H_INCLUDED