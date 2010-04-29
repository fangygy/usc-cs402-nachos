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
  Print("Created lock with name \"lockOne\"\n",0,0,0);
  
  Print("Acquiring lock0\n",0,0,0);
  Acquire(lock0);
  Print("Acquired lock0\n",0,0,0);

  Print("Creating MV with name \"mvOne\"\n",0,0,0);
  mv0 = CreateMV("mvOne");
  Print("Created MV with name \"mvOne\" and ID=%d\n",mv0,0,0);

  
  mv0value = GetMV(mv0);
  Print("Current value of \"mvOne\" =%d\n",mv0value,0,0);

  mv0value++;

  Print("Setting value of \"mvOne\" to %d\n",mv0value,0,0);
  SetMV(mv0,mv0value);

  mv0value = GetMV(mv0);
  Print("New value of \"mvOne\" =%d\n",mv0value,0,0);
  
  Print("Releasing lock0\n",0,0,0);
  Release(lock0);

}
