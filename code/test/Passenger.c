#include "functions.h"
#include "syscall.h"

#ifdef CHANGED
#include "synch.h"
#endif

/*PASSENGER*/
void main(int passengerNum) {
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
    int myLineNumber;
    int myLineCount;

    int alLock;

    /* Create CVS for interactions with AL, CIS, SO, SI */
    int waitingForAL_C;

    passengerNumLock = createLock("passengerNumLock");
    Acquire(passengerNumLock);

    myNumber = passengerNum;
    passengerNum++;
    Release(passengerNumLock);


    /* --------------------------------------------------------
       1. Passenger goes to see Airport Liaison
       
       -------------------------------------------------------- */

    /* Passenger acquires the lock so they can search for shortest line amongst all lines */
    alLineLock = createLock("alLineLock");
    Acquire(alLineLock);

    /* Get the indexes to the MV to the Airport Liaison Line */
    al1_line = createMV("al_line1");
    al2_line = createMV("al_line2");
    al3_line = createMV("al_line3");

    /* Get the values to the MV of the Aiport Liaison Lines */
    al1_line_length = getMV(al1_line);
    al2_line_length = getMV(al2_line);
    al3_line_length = getMV(al3_line);

    /* Store these in an array so that we can find minimum */
    lineLengths[0] = al1_line_length;
    lineLengths[1] = al2_line_length;
    lineLengths[2] = al3_line_length;

    /* Now check to see who has the shortest line */
    shortLine = findShortestLine(lineLengths,3);
    
    /* Get the index for the Airport Liaison Line */
    /* Also set the lineCount as we will use this later to set the MV value */
    switch(shortLine){
    case 0:
      myLineNumber   = al1_line;
      myLineCount    = al1_line_length;
      waitingForAL_C = createCondition("waitAL1");
      alLock         = createLock("alLock1");
      waitingForTicket_AL_C = createCondition("waitTicketAL1");
      /* al_busy = createMV("al1_busy"); */
      break;
    case 1:
      myLineNumber = al2_line;
      myLineCount  = al2_line_length;
      waitingForAL_C = createCondition("waitAL2");
      alLock         = createLock("alLock2");
      waitingForTicket_AL_C = createCondition("waitTicketAL2");
      /* al_busy = createMV("al2_busy"); */
      break;
    case 2:
      myLineNumber = al3_line;
      myLineCount  = al3_line_length;
      waitingForAL_C = createCondition("waitAL3");
      alLock         = createLock("alLock3");
      waitingForTicket_AL_C = createCondition("waitTicketAL3");
      /* al_busy = createMV("al3_busy"); */
      break;
    }
   
    /* Get in the line */

    /* Increment the line count */
    myLineCount++;
    setMV(myLineNumber, myLineCount); 
 
    Wait(waitingForAL_C, alLineLock);   
    Release(alLineLock);


    Acquire(alLock);

    /* decrement line count (left the line) */
    myLineCount = getMV(myLineNumber) - 1;
    setMV(myLineNumber, myLineCount);
        
    /* Passenger is told to go to counter, and hands their ticket to Liaison */
    Signal(waitingForTicket_AL_C, alLock);
    Wait(waitingForTicket_AL_C, alLock);
    
    Release(alLock);

    /*Exit for now...*/
    Exit(0);
}
