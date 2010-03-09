/*
 * Simple file to test Exec and Exit.
 *
 *
*/

#include "syscall.h"


int main() {
  int result;
  result = 1;

  Write("Beginning Exec test\n",21,ConsoleOutput);

  result = Exec("../test/exittest");
  /*Exec("../test/exittest");*/

  Write("After Exec\n",13,ConsoleOutput);


  Exit(result); /*should exit with status=10 if Exec worked properly*/
}
