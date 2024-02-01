/* Holds code for appending to, removing from and navigating the variable
 * buffer. */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

// The default size of the buffer.
#define BUFFER_SIZE 500

/* Allocates a buffer of `n` length where each element's value is set to `0`. */
extern inline char *make_buffer(int n) {
  if (n < 1) {
    n = BUFFER_SIZE;
  }

  char *buf = (char *)malloc(sizeof(char) * n);

  // Fill with null bytes
  for (int i = 0; i < n; i++) {
    buf[i] = 0;
  }

  return buf;
}

#endif
