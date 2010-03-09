/*
 * Simple file to test the Exit syscall.
 */

#include "syscall.h"

int main() {
  Write("Beginning Exit test\n",21,ConsoleOutput);
  /* There should be only one exit call (with status=10) */
  Exit(10);
  /* If there is an exit syscall with status=9, the first exit didn't work */
  Exit(9);
}
