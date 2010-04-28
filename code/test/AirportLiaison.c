#include "functions.h"
#include "syscall.h"

#ifdef CHANGED
#include "synch.h"
#endif

/*AIRPORT LIAISON */
int main(int airportLNum) {
  int myNumber;
  int waitingForAL_C;
  int waitingForTicket_AL_C;
  int airportNumLock; 
  int myLine; 
  int alPassengerCount;
  int passCount;
  int alLock;
  int alLineLock;

  /* Use this as a buffer for string / int concatenation */
  char buff[50];
 
  char *WFCname = "waitAl";
  char *Linename = "al_line";
  char *WFTCname = "waitTicketAL";
  char *ALLname = "alLock";
  
  airportNumLock = CreateLock("airportNumLock");
  Acquire(airportNumLock);

  myNumber = airportLNum;
  airportLNum++;
  Release(airportNumLock);

  
  WFCname[7]   = (char)myNumber;
  WFTCname[12] = (char)myNumber;
  Linename[12] = (char)myNumber;
  ALLname[7]   = (char)myNumber;
  

  waitingForAL_C = CreateCondition(WFCname);
  waitingForTicket_AL_C = CreateCondition(WFTCname);
  myLine         = CreateMV(Linename);
  alLock         = CreateLock(ALLname);

  while(1) {
    int flight_number = 0;
    /* Acquire the Line Lock
     No one can acquire the line lock (not Passengers)
     When the Airport Liaison has the LineLock, Passengers cannot search for the shortest
     line */
    /*pass name to try creating "same" lock for same critical region*/
    alLineLock = CreateLock("alLineLock");
    Acquire(alLineLock);

    /* If there are passengers in the line, then
     the Airport Liaison must tell the Passengerto step up to the counter
     He does this by Signaling the Condition Variable which puts the first Passenger
     on to the Ready Queue */
    
    /* getMV(myLine) will give us the line count */
    if(GetMV(myLine)>0) {
      /* The first passenger waiting for the LineLock gets put on to the Ready Queue */
      Signal(waitingForAL_C, alLineLock);
    } else {
      /* Airport Liaison is not busy if there is no one in line */
      /* al_busy[myNumber] = 0; */
    }
    
    /* Acquire the lock to the Airport Liaison
     We will use this lock to control the interactions between the Passenger
     and the Airport Liaison */

    Acquire(alLock);
    
    /* After acquiring that lock, we release the Line Lock so who ever is waiting for the 
       Line Lock can then search for the shortest line and then get into the appropriate line */
    Release(alLineLock);
    
    /* The Airport Liaison must now wait for the Passenger to go up to their counter 
     and give them their ticket 
     Sleeping the Airport Liaison frees up the alLock, wakes up one Passenger and puts them on the 
     Ready Queue */
    Wait(waitingForTicket_AL_C, alLock);
    
    /* Count the passenger's baggage?? */
    
    Signal(waitingForTicket_AL_C, alLock);
    
    alPassengerCount = CreateMV("alPassengerCount");
    passCount = GetMV(alPassengerCount) + 1;
    SetMV(alPassengerCount, passCount);
    
    Release(alLock);
  }
}
