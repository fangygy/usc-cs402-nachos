#include "syscall.h"

int main () {

  int lock0 = -1;
  int mv0 = -1;
  int mv0value = -1;

  Print("Registering with network thread.\n",0,0,0);
  Register();
  Print("Done Registering with network thread.\n",0,0,0);

  Print("Creating lock with name \"lockOne\"\n",0,0,0);
  lock0 = CreateLock("lockOne");
  Print("Created lock with name \"lockOne\" and ID=%d\n",lock0,0,0);
  
  Print("Acquiring lock ID=%d\n",lock0,0,0);
  Acquire(lock0);
  Print("Acquired lock ID=%d\n",lock0,0,0);

  Print("Creating MV with name \"mvOne\"\n",0,0,0);
  mv0 = CreateMV("mvOne");
  Print("Created MV with name \"mvOne\" and ID=%d\n",mv0,0,0);

  
  mv0value = GetMV(mv0);
  Print("Initial value of \"mvOne\"\n",mv0value,0,0);

  Print("Setting \"mvOne\"'s value to %d\n",77,0,0);
  SetMV(mv0,77);

  mv0value = GetMV(mv0);
  Print("New value of \"mvOne\"=%d\n",mv0value,0,0);

  Print("Releasing lock ID=%d\n",lock0,0,0);
  Release(lock0);
  
  

}
