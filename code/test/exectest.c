/*
 * Simple file to test Exec and Exit.
 *
 *
*/

#include "syscall.h"


int main() {

  Print("Beginning Virtual Memory Test\n",0,0,0);
  Exec("../test/halt");
  Exec("../test/halt");
  Exit(0); /*should exit with status=10 if Exec worked properly*/
}
