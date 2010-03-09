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
  /*/Test: Create invalid condition
  //test name too small
  //test name too large
  //test condition created outside address space*/

  /*/create valid lock*/
  int bogus;
  lock1 = CreateLock(1);
  /*/create valid condition*/
  cond1 = CreateCondition(1);

  /*/Test: too many conditions created*/
  while(1){
    int cond = CreateCondition(1);
    if(cond == -1)
      break;
  }
  
  /*/Test: destroy condition in use/
  //wait valid lock(SUCCESS)*/
  Fork(waitFunction);
  /*/destroy condition(FAIL)
  DestroyCondition(cond1);
  /*destroy lock(FAIL)
  DestroyLock(lock1-1);
  /*wait lock twice more*/
  Fork(waitFunction);
  Fork(waitFunction);
  /*/signal lock(SUCCESS)*/
  Signal(cond1,lock1);
  /*/destroy condition(FAIL)
  DestroyCondition(cond1);
  /*destroy lock(FAIL)
  DestroyLock(lock1);
  /*broadcast lock(SUCCESS)*/
  Broadcast(cond1,lock1);
  /*/destroy condition(SUCCESS)*/
  DestroyCondition(cond1);
  /*/destroy lock(SUCCESS)*/
  DestroyLock(lock1);

  bogus = 235201;
  /*/Test: wait/signal/broadcast with bogus condition(FAIL)*/
  Wait(bogus,lock1);
  Signal(bogus,lock1);
  Broadcast(bogus,lock1);
  /*/Test: wait/signal/broadcast destroyed condition(FAIL)
  Wait("\0",lock1);
  Signal("\0",lock1);
  Broadcast("\0",lock1);
  /*Test: wait/signal/broadcast bogus lock(FAIL)*/
  Wait(cond1,bogus);
  Signal(cond1,bogus);
  Broadcast(cond1,bogus);
  /*/Test: wait/signal/broadcast using destroyed lock(FAIL)
  Wait(cond1,"\0");
  Signal(cond1,"\0");
  Broadcast(cond1,"\0");
  /*Test: out of bounds...
  //Test: belongs to another process...*/
}
