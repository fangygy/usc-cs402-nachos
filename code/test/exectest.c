/*
 * Simple file to test Exec and Exit.
 *
 *
*/

#include "syscall.h"


int main() {

  Print("Beginning Virtual Memory Test\n",0,0,0);
  Exec("../test/matmult");
  Exec("../test/matmult");
  Exit(0); /*should exit with status=10 if Exec worked properly*/
}
