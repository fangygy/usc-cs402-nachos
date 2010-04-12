/* testLocks.c
 *	Simple program to test the lock syscalls
 */

#include "syscall.h"

int main() {
  /*/Test: Create invalid lock
  //test name too small
  //test name too large
  //test lock created outside address space
  
  //create valid lock*/
  int bogus;
  int lock1, lock2;

  lock1 = CreateLock("abc");
  lock2 = CreateLock("bc");
  /*/Test: too many locks created(FAIL)*/
  /*KTwhile(1){
    int lock = CreateLock("a");
    Print("Received: %d\n",lock,0,0);
    if(lock == -1) {
      Write("rec -1",6,1);
      break;
    }
    }*/

  
  /*/Test: destroy lock in use
  //acquire valid lock(SUCCESS)*/
  Acquire(lock1);
  /*KTAcquire(lock2);*/
  /*/destroy lock(FAIL)*/
  DestroyLock(lock1-1);
  /*/release lock(SUCCESS)*/
  /*KT Release(lock1);*/
  /*/destroy lock(SUCCESS)*/
  /*KT DestroyLock(lock1);*/
  /*/Test: acquire destroyed lock(FAIL)*/
  /*KT Acquire(lock1);*/
  /*/Test: release destroyed lock(FAIL)*/
  /*KT Release(lock1);*/

  /*KT bogus = 235201;*/
  /*/Test: aquire/release bogus lock(FAILx2)*/
  /*KT Acquire(bogus); */
  /*KT Release(bogus);*/
  /*/Test: aquire/release destroyed lock(FAILx2)(((already done...)))
  Acquire("\0");
  Release("\0");*/
  /*/Test: out of bounds...
  //Test: belongs to another process...*/
}
