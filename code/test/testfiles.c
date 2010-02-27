/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int main() {
  OpenFileId fd;
  int bytesread;
  char buf[20];
  int lockid;
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
  Write("testing a lock\n", 16, ConsoleOutput);
  lockid = CreateLock();
  lockid = CreateLock();
  lockid = CreateLock();
  for(int i = 0; i < lockid; i++) {
    Write("number of lock id\n", 32, ConsoleOutput);
  }
  Write("testing a condition \n", 16, ConsoleOutput);
}
