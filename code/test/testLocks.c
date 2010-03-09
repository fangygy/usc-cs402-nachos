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
  while(1){
    int lock = CreateLock("a");
    if(lock == -1)
      break;
  }
  
  /*/Test: destroy lock in use
  //acquire valid lock(SUCCESS)*/
  Acquire(lock1);
  /*/destroy lock(FAIL)*/
  DestroyLock(lock1);
  /*/release lock(SUCCESS)*/
  Release(lock1);
  /*/destroy lock(SUCCESS)*/
  DestroyLock(lock1);
  /*/Test: acquire destroyed lock(FAIL)*/
  Acquire(lock1);
  /*/Test: release destroyed lock(FAIL)*/
  Release(lock1);

  bogus = 235201;
  /*/Test: aquire/release bogus lock(FAILx2)*/
  Acquire(bogus);
  Release(bogus);
  /*/Test: aquire/release destroyed lock(FAILx2)*/
  Acquire("\0");
  Release("\0");
  /*/Test: out of bounds...
  //Test: belongs to another process...*/
}
