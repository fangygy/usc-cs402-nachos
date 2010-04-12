/* lockTest2.c
 * tests locks for client #2
 */

#include "syscall.h"

int main() {

  
  int lock0 = 0;
  int lock1 = 1;
  int lock2 = CreateLock("xxx"); /*should be 2 if client0 creates lock0 and lock1 first*/

  int cv0 = 0;

  Print("Client2 about to acquire lock0.\n",0,0,0);
  Acquire(lock0);
  Print("Client2 acquired lock0.\n",0,0,0);

  Print("Client2 about to signal cv0 lock0.\n",0,0,0);
  Signal(cv0, lock0); /*this should wake up client1*/
  Print("Client2 signaled cv0 lock0.\n",0,0,0);

  Print("Client2 about to release lock0.\n",0,0,0);
  Release(lock0); 
  Print("Client2 released lock0.\n",0,0,0);

  Print("Client2 about to acquire lock1.\n",0,0,0);
  Acquire(lock1); /* should wait here until client1 wakes up and releases*/
  Print("Client2 acquired lock1.\n",0,0,0);

  Print("Client2 about to release lock1.\n",0,0,0);
  Release(lock1); 
  Print("Client2 released lock1.\n",0,0,0);

  


}
