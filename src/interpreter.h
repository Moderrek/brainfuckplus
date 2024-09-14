#ifndef INTERPRETER_H_INCLUDED
#define INTERPRETER_H_INCLUDED

#include <stdbool.h>

#include "state.h"

void action_breakpoint(BFData* data);
bool interpret(BFData* data, const char* code, const char* filename);
bool interpret_file(BFData* data, const char* filename);

#endif // INTERPRETER_H_INCLUDED