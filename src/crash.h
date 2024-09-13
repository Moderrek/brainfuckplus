#ifndef CRASH_H_INCLUDED
#define CRASH_H_INCLUDED

#include <stdio.h>

#include "state.h"

void crash(BFData* data, const char* message, int code) {
  fprintf(stderr, "ERROR: program crashed with code %d because: %s\n", code, message);
  if (data->memdump)
    print_mem(data);
  exit(code);
}

void crash_file(BFData* data, const char* message, int code, const char* filename, int pos) {
  fprintf(stderr, "%s:%d: ERROR: %s\n", filename, pos+1, message);
  fprintf(stderr, "Program crashed with exit code %d\n", code);
  if (data->memdump)
    print_mem(data);
  exit(code);
}

#endif // CRASH_H_INCLUDED