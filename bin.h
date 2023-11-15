/* Functions for manipulating and interpreting binary data. */

#ifndef BIN_H
#define BIN_H

#include <stdio.h>
#include <stdlib.h>

/* Converts `bin` (a series of 0s or 1s) to the character it represents. */
char interpret_bin(const char *bin) { return (char)strtol(bin, 0, 2); }

/* Takes `n`, a character, and converts it to it's binary
 * representation (`b` bits). */
char *dump_bin(char n, int b) {
  char *bin = (char *)malloc(b + 1);
  bin[b] = '\0';

  for (int i = b - 1; i >= 0; i--) {
    bin[i] = (n & 1) + '0';
    n >>= 1;
  }

  return bin;
}

#endif
