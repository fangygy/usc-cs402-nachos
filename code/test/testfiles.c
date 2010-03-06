/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int main() {
  OpenFileId fd;
  int bytesread;
  char buf[20];
  int lockid, condid;
  /*
    Create("testfile", 8);
    fd = Open("testfile", 8);

    Write("testing a write\n", 16, fd );
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);
  */
  /*
  Write("testing a lock\n", 16, ConsoleOutput);
  lockid = CreateLock();
  Acquire(lockid);
  
  condid = CreateCondition();
  Wait(condid, lockid);
	
  
   Write a test suite here that tests all the syscalls 
   The ones above test Acquire and Wait
   */

  Write("testing exec\n",16,ConsoleOutput);
  
  
  Exec("helloworld");

  
  Write("tests finished \n", 32, ConsoleOutput);
}
