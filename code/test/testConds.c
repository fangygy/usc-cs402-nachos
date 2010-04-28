/* testConds.c
 *	Simple program to test the condition syscalls
 */

#include "syscall.h"

int lock1;
int cond1;

void waitFunction(){
  Wait(cond1,lock1);
  Exit(0);
}



int main() {
  
}
