/* lockTest1.c
 * tests locks for client #1
 */

#include "syscall.h"

int main() {

  
  int lock0 = CreateLock("aaa"); /*should be 0 if this client runs first*/
  int cv0 = CreateCondition("cv");

  int lock1 = 1;
  
  Print("Client1 about to acquire lock0.\n",0,0,0);
  Acquire(lock0);
  Print("Client1 acquired lock0.\n",0,0,0);

  Print("Client1 about to acquire lock1.\n",0,0,0);
  Acquire(lock1);
  Print("Client1 acquired lock1.\n",0,0,0);

  Print("Client1 about to wait cv0 lock0.\n",0,0,0);
  Wait(cv0, lock0); /*should wait here until client2 signals*/
  Print("Client1 after wait cv0 lock0.\n",0,0,0);

  Print("Client1 about to release lock0.\n",0,0,0);
  Release(lock0);
  Print("Client1 released lock0.\n",0,0,0);

  Print("Client1 about to release lock1.\n",0,0,0);
  Release(lock1);
  Print("Client1 released lock1.\n",0,0,0);

  


}
