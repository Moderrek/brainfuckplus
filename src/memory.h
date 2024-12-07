#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED 1

#include <stdbool.h>

#include "state.h"

#define BF_CAPACITY 30000 /*30kB*/
#define BF_LOOP_STACK 1024 /* Max amount of nested loops */

#define CLEAR_CURR_LINE "\033[2K"
#define LINE_UP "\033[F"

void print_mem(const BFData* data);
void hide_mem(const BFData* data);
bool valid_ptr(const BFData* data);
void dump_mem(const BFData* data, const char* filename);

#endif // MEMORY_H_INCLUDED