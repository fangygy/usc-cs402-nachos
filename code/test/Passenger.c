#include "functions.h"
#include "syscall.h"

#ifdef CHANGED
#include "synch.h"
#endif

/*PASSENGER*/
int main(int passengerNum) {
    int myLineNumber;

    int myFlightNumber;
    int amExecutive;

    int checkin_counter_number;
    int start, stop;
    int myNumber;
    int al_busy;
    int busy;/*bool*/
    
    int alLines = 3;
    int cisLines = 3; 
    int solines = 3;
    int siLines = 3; 

    int al1_line, al2_line, al3_line;
    int al1_line_length, al2_line_length, al3_line_length;
    int lineLengths[3];
    int shortLine;
    int myLineCount;

    /* Locks */
    int passengerNumLock;
    int alLock;
    int alLineLock;

    /* Create CVS for interactions with AL, CIS, SO, SI */
    int waitingForAL_C;
    int waitingForTicket_AL_C;

    /* Create MVs*/
    int passNumMV;
    int passNumMV_v;
    

    Register();
    Print("Registered Passenger\n",0,0,0);

    passengerNumLock = CreateLock("passengerNumLock");
    Acquire(passengerNumLock);

    myNumber = currentThread->getMailbox();
    Release(passengerNumLock);

    Print("Passenger Number is: %d \n", myNumber,0,0);

    /* --------------------------------------------------------
       1. Passenger goes to see Airport Liaison
       
       -------------------------------------------------------- */

    /* Passenger acquires the lock so they can search for shortest line amongst all lines */
    alLineLock = CreateLock("alLineLock");
    Acquire(alLineLock);

    /* Now check to see who has the shortest line */
    myLineNumber = myNumber;

    Print("Passenger %d chose Airport Liaison Line %d \n",myNumber ,shortLine, 0);

    /* Get the index for the Airport Liaison Line */
    /* Also set the lineCount as we will use this later to set the MV value */
    switch(myLineNumber){
    case 0:
      myLineCount    = al1_line_length;
      waitingForAL_C = CreateCondition("waitAL1");
      alLock         = CreateLock("alLock1");
      waitingForTicket_AL_C = CreateCondition("waitTicketAL1");
      /* al_busy = createMV("al1_busy"); */
      break;
    case 1:
      myLineCount  = al2_line_length;
      waitingForAL_C = CreateCondition("waitAL2");
      alLock         = CreateLock("alLock2");
      waitingForTicket_AL_C = CreateCondition("waitTicketAL2");
      /* al_busy = createMV("al2_busy"); */
      break;
    case 2:
      myLineCount  = al3_line_length;
      waitingForAL_C = CreateCondition("waitAL3");
      alLock         = CreateLock("alLock3");
      waitingForTicket_AL_C = CreateCondition("waitTicketAL3");
      /* al_busy = createMV("al3_busy"); */
      break;
    }
   
    /* Get in the line */
    
    Wait(waitingForAL_C, alLineLock);   
    Release(alLineLock);

    Acquire(alLock);

    /* decrement line count (left the line) */
        
    /* Passenger is told to go to counter, and hands their ticket to Liaison */
    Signal(waitingForTicket_AL_C, alLock);
    Wait(waitingForTicket_AL_C, alLock);
    
    Release(alLock);
    Print("Passenger %d going to Check in Staff %d: \n",myNumber, myLineNumber,0);
    /*Exit for now...*/
    Exit(0);
}
