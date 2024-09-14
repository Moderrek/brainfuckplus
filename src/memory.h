#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <stdbool.h>

#include "state.h"

#define BF_CAPACITY 30000 /*30kB*/
#define BF_LOOP_STACK 1024 /* Max amount of nested loops */

#define CLEAR_CURR_LINE "\033[2K"
#define LINE_UP "\033[F"

void print_mem(BFData* data);
void hide_mem(BFData* data);
bool valid_ptr(BFData* data);

#endif // MEMORY_H_INCLUDED