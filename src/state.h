#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <stdbool.h>

typedef struct {
  unsigned char* memory;
  int* loop_stack;

  int loop_ptr;
  int mem_ptr;
  int max_used_ptr;

  bool debug;
  bool warnings;
  bool memdump;
  char* memdump_file;
} BFData;

#endif