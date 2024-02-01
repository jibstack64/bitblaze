/* Utilities for creating pointers easily. */

#ifndef POINTER_H
#define POINTER_H

#include <stdlib.h>

/* Allocates memory for `n`. */
extern inline int *make_pointer(int n) {
  int *np = (int *)malloc(sizeof(int));
  np[0] = n;
  return np;
}

#endif
