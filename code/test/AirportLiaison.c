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
  char WFCname[6];
  char WFTCname[12];
  char ALLname[6];
  int airportNumLock; 
  int myLine; 
  int alPassenterCount;
  int passCount;

  WFCname = "waitAL";
  WFTCname = "waitTicketAL";
  ALLname = "alLock";
  
  airportNumLock = createLock("airportNumLock");
  Acquire(airportLNumLock);

  myNumber = airportLNum;
  airportLNum++;
  Release(airportNumLock);

  waitingForAL_C = createCondition(strcat(WFCname,myNumber));
  waitingForTicket_AL_C = createCondition(strcat(WFTCname,myNumber));
  myLine         = createMV(strcat("al_line",myNumber));
  alLock         = creatLock(strcat(ALLname,myNumber));

  while(1) {
    int flight_number = 0;
    /* Acquire the Line Lock
     No one can acquire the line lock (not Passengers)
     When the Airport Liaison has the LineLock, Passengers cannot search for the shortest
     line */
    /*pass name to try creating "same" lock for same critical region*/
    alLineLock = createLock("alLineLock");
    Acquire(alLineLock);

    /* If there are passengers in the line, then
     the Airport Liaison must tell the Passengerto step up to the counter
     He does this by Signaling the Condition Variable which puts the first Passenger
     on to the Ready Queue */
    
    /* getMV(myLine) will give us the line count */
    if(getMV(myLine)>0) {
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
    
    alPassenterCount = createMV("alPassengerCount");
    passCount = getMV(alPassengerCount) + 1;
    setMV(alPassengerCount, passCount);
    
    Release(alLock);
  }
}
