#ifndef INTERPRETER_H_INCLUDED
#define INTERPRETER_H_INCLUDED

#include <stdbool.h>

#include "state.h"

bool interpret(BFData* data, const char* code, const char* filename);
bool interpret_file(BFData* data, const char* filename);

#endif // INTERPRETER_H_INCLUDED