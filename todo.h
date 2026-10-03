/* todo.h -- marks a function that you still have to write (starter code only). */
#ifndef TODO_H
#define TODO_H
#include <stdio.h>
#include <stdlib.h>
#define TODO(fn) do { fflush(stdout); fprintf(stderr, "TODO: %s is not implemented yet\n", fn); exit(3); } while (0)
#endif
