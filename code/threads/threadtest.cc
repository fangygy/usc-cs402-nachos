// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

// Authors Alex Lee

#include "copyright.h"
#include "system.h"
#include "functions.h"
// #include "testsuite.h"
#ifdef CHANGED
#include "synch.h"
#endif

// ---------------------------------------------------------------------
/*
 * PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO 
 * RUN AIRPORT SIMULATIONS
 *
 *
 *
*/
// ---------------------------------------------------------------------

// Global variables

#define numberOfPassengers 20
#define numberOfAL 7
#define numberOfCIS 15
#define numberOfSO 7
#define numberOfAirlines 3

int sicount = 1;
int socount = 1;
int pass_si_count = 1;

struct ticket {
  int passenger_number;
  int flight_number;
  int checkin_counter;
} pass_ticket_buffer[numberOfPassengers];

struct boarding_pass {
  int passenger_number;
  int flight_number;
  int seat_number;
} boarding_pass_buffer[numberOfPassengers];

int al_current_passenger_serving[7]; // must be equal to the number of airport liaisons


void AirportManager(int myNumber) {

}

void CargoHandler(int myNumber) {

}

Condition *waitingForSI_C[numberOfSO];
Condition *waitingForTicket_SI_C[numberOfSO];
Lock siLineLock("si_LL");
Lock *siLock[numberOfSO];
int siLineLengths[numberOfSO];
bool si_busy[numberOfSO];


void SecurityInspector(int myNumber) {
  while(true) {
    
    siLineLock.Acquire();
    
    if(siLineLengths[myNumber]>0) {
      printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSI_C[myNumber]->Signal(&siLineLock);
    } else {
      waitingForSI_C[myNumber]->Wait(&siLineLock);
      printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSI_C[myNumber]->Signal(&siLineLock);
    }
    
    siLock[myNumber]->Acquire();
    siLineLock.Release();
    
    waitingForTicket_SI_C[myNumber]->Wait(siLock[myNumber]);
    waitingForTicket_SI_C[myNumber]->Signal(siLock[myNumber]);
    // Clear passenger and direct to Security Inspector
    printf("%s: moving Passenger to Boarding: passengers moved: %d \n", currentThread->getName(), sicount);
    sicount++;
    siLock[myNumber]->Release();
  }
}

Condition *waitingForSO_C[numberOfSO];
Condition *waitingForTicket_SO_C[numberOfSO];
Lock soLineLock("sl_LL");
Lock *soLock[numberOfSO];
int soLineLengths[numberOfSO];
bool so_busy[numberOfSO];

void SecurityOfficer(int myNumber) {
  while(true) {
    
    soLineLock.Acquire();
    
    if(soLineLengths[myNumber]>0) {
      printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSO_C[myNumber]->Signal(&soLineLock);
    } else {
      waitingForSO_C[myNumber]->Wait(&soLineLock);
      printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSO_C[myNumber]->Signal(&soLineLock);
    }
    
    soLock[myNumber]->Acquire();
    soLineLock.Release();
    
    waitingForTicket_SO_C[myNumber]->Wait(soLock[myNumber]);
    waitingForTicket_SO_C[myNumber]->Signal(soLock[myNumber]);
    // Clear passenger and direct to Security Inspector
    printf("%s: moving Passenger to Security Inspector: number of Passengers moved: %d\n", currentThread->getName(),socount);
    socount++;
    soLock[myNumber]->Release();
  }
}

// Objects for Airport Liaison
Condition *waitingForAL_C[numberOfAL];
Condition *waitingForTicket_AL_C[numberOfAL];
Lock alLineLock("al_LL");
Lock *alLock[numberOfAL];
int alLineLengths[numberOfAL];
bool al_busy[numberOfAL];
int alPassenger[numberOfAL];

void AirportLiaison(int myNumber) {
  while(true) {
    // Acquire the Line Lock
    // No one can acquire the line lock (not Passengers)
    // When the Airport Liaison has the LineLock, Passengers cannot search for the shortest
    // line
    alLineLock.Acquire();

    // If there are passengers in the line, then
    // the Airport Liaison must tell the Passenger to step up to the counter
    // He does this by Signaling the Condition Variable which puts the first Passenger
    // on to the Ready Queue
    if(alLineLengths[myNumber]>0) {
      printf("%s telling Passenger to step up to counter\n",currentThread->getName());
      // The first passenger waiting for the LineLock gets put on to the Ready Queue
      waitingForAL_C[myNumber]->Signal(&alLineLock);
    } else {
      // Airport Liaison is not busy if there is no one in line
      al_busy[myNumber] = false;
    }
    
    // Acquire the lock to the Airport Liaison
    // We will use this lock to control the interactions between the Passenger
    // and the Airport Liaison
    alLock[myNumber]->Acquire();
    
    // After acquiring that lock, we release the Line Lock so who ever is waiting for the 
    // Line Lock can then search for the shortest line and then get into the appropriate line 
    alLineLock.Release();
    
    // The Airport Liaison must now wait for the Passenger to go up to their counter 
    // and give them their ticket 
    // Sleeping the Airport Liaison frees up the alLock, wakes up one Passenger and puts them on the 
    // Ready Queue 
    waitingForTicket_AL_C[myNumber]->Wait(alLock[myNumber]);
    
    // The Airport Liaison signals a Passenger, who is asleep waiting for the Airport
    // Liaison to tell them where to go
    waitingForTicket_AL_C[myNumber]->Signal(alLock[myNumber]);
    pass_ticket_buffer[alPassenger[myNumber]].checkin_counter = pass_ticket_buffer[alPassenger[myNumber]].flight_number;  
    printf("%s: Directing Passenger%d to Airline check in counter %d\n", currentThread->getName(), alPassenger[myNumber], pass_ticket_buffer[alPassenger[myNumber]].checkin_counter);
    alLock[myNumber]->Release();
  }
}


// Objects for Check In Staff
Condition *waitingForCIS_C[numberOfCIS];
Condition *waitingForTicket_CIS_C[numberOfCIS];
Condition *onBreakCIS_C[numberOfCIS];
Lock *cisLineLock[numberOfAirlines];
Lock *cisLock[numberOfCIS];
int cisLineLengths[numberOfCIS];
bool cis_busy[numberOfCIS];


void CheckInStaff(int myNumber) {
  while(true) {
    int myAirline;
    if(myNumber > 9) {
      myAirline = 2;
    } else if (myNumber > 4) {
      myAirline = 1;
    } else {
      myAirline = 0;
    }
    
    // Acquire the line lock
    cisLineLock[myAirline]->Acquire();

    /*
     * if there is no one in line 
     * check in staff goes on break
     *
     * there are people in line 
     * signal someone to come into the line
     * wait for them to hand their ticket
     * generate boarding pass
     * signal passenger with boarding pass
     * 
     */

    if(cisLineLengths[myNumber]==0) {
      // go on break
      onBreakCIS_C[myNumber]->Wait(cisLineLock[myAirline]);
    }
    
    // If executive line > 0
    // Help the executives
    if(cisLineLengths[myNumber] > 0) {
      //printf("line %d has more than one passenger\n", myNumber);
      waitingForCIS_C[myNumber]->Signal(cisLineLock[myAirline]);
      printf("%s telling Passenger to come to counter\n", currentThread->getName());
    }

    cisLock[myNumber]->Acquire();
    cisLineLock[myAirline]->Release();
    waitingForTicket_CIS_C[myNumber]->Wait(cisLock[myNumber]);
    waitingForTicket_CIS_C[myNumber]->Signal(cisLock[myNumber]);
    
    printf("%s giving Passenger ticket number and directing them to gate\n", currentThread->getName());
    cisLock[myNumber]->Release();
    
  }
}

void Passenger(int myNumber) {

  // --------------------------------------------------------
  // 1. Passenger goes to see Airport Liaison
  //
  //
  // --------------------------------------------------------

  // Passenger acquires the lock so they can search for shortest line amongst all lines
  alLineLock.Acquire();

  // Declare the variable for the Passenger's line number
  // We will reuse this variable for all 
  int myLineNumber;

  // Set the Passenger's Line number
  printf("%s: Searching for the shortest line\n", currentThread->getName());
  myLineNumber = findShortestLine(alLineLengths,7);
  
  // If there are people in the line, or the Airport Liaison is busy
  // then the Passenger must wait in line and NOT approach the Airport Liaison
  if((alLineLengths[myLineNumber] > 0)||(al_busy[myLineNumber])) {
    alLineLengths[myLineNumber]++;
    printf("%s chose Liaison %d  with a line of length %d\n",currentThread->getName(),myLineNumber,alLineLengths[myLineNumber]);
    waitingForAL_C[myLineNumber]->Wait(&alLineLock);
    al_busy[myLineNumber] == true;
  }

  alLineLock.Release();
  alLock[myLineNumber]->Acquire();

  printf("%s: Going to see Liaison %d\n",currentThread->getName(),myLineNumber);
  alLineLengths[myLineNumber]--;

  // Passenger is told to go to counter, and hands their ticket to Liaison
  alPassenger[myLineNumber] = myNumber;
  waitingForTicket_AL_C[myLineNumber]->Signal(alLock[myLineNumber]);
  waitingForTicket_AL_C[myLineNumber]->Wait(alLock[myLineNumber]);

  alLock[myLineNumber]->Release();

  // --------------------------------------------------------
  // 2. Passenger goes to see Airport check in staff
  //
  //
  // --------------------------------------------------------

  // Acquire the Lock to the line
  // Only use one lock for all 5 lines, because only one Passenger at 
  // a time can be looking for the shortest line 
  int checkin_counter_number = pass_ticket_buffer[myNumber].checkin_counter;
  
  cisLineLock[checkin_counter_number]->Acquire();

  int start, stop;
  start = (pass_ticket_buffer[myNumber].checkin_counter)*(numberOfCIS/3);
  stop  = start + (numberOfCIS/3) - 1;
  // Figure out which 

  // Set the Passenger's line number
  myLineNumber = findCISShortestLine(cisLineLengths,start,stop);

  cisLineLengths[myLineNumber]++;
  onBreakCIS_C[myLineNumber]->Signal(cisLineLock[checkin_counter_number]);
  printf("%s chose Airline Check In %d with length %d\n", currentThread->getName(), myLineNumber, cisLineLengths[myLineNumber]);
  waitingForCIS_C[myLineNumber]->Wait(cisLineLock[checkin_counter_number]);
  cisLineLengths[myLineNumber]--;

  printf("%s going to see Airline Check In Staff %d\n",currentThread->getName(), myLineNumber); 
  cisLineLock[checkin_counter_number]->Release();
  cisLock[myLineNumber]->Acquire();
  

  // The Passenger now has the line number, so they should go to sleep and
  // release the line lock, letting another Passenger search for a line
  printf("%s giving airline ticket to Airline Check In Staff %d\n", currentThread->getName(), myLineNumber);
  waitingForTicket_CIS_C[myLineNumber]->Signal(cisLock[myLineNumber]);
  waitingForTicket_CIS_C[myLineNumber]->Wait(cisLock[myLineNumber]);
  cisLock[myLineNumber]->Release();

  // --------------------------------------------------------
  // 3. Passenger goes to see Airport Security Officer
  //
  //
  // --------------------------------------------------------
  
  soLineLock.Acquire();
  
  myLineNumber = findShortestLine(soLineLengths, 7);

  soLineLengths[myLineNumber]++;
  printf("%s: chose Security %d with length %d\n", currentThread->getName(), myLineNumber, soLineLengths[myLineNumber]);
  waitingForSO_C[myLineNumber]->Signal(&soLineLock);
  waitingForSO_C[myLineNumber]->Wait(&soLineLock);
  
  soLineLengths[myLineNumber]--;
  soLineLock.Release();

  soLock[myLineNumber]->Acquire();

 // The Passenger now has the line number, so they should go to sleep and
  // release the line lock, letting another Passenger search for a line
  printf("%s giving airline ticket to Security Officer %d\n", currentThread->getName(), myLineNumber);
  waitingForTicket_SO_C[myLineNumber]->Signal(soLock[myLineNumber]);
  waitingForTicket_SO_C[myLineNumber]->Wait(soLock[myLineNumber]);
  soLock[myLineNumber]->Release();

  // --------------------------------------------------------
  // 4. Passenger goes to see Airport Security Inspector
  //
  //
  // --------------------------------------------------------
  
  siLineLock.Acquire();
  
  myLineNumber = findShortestLine(siLineLengths, 7);

  siLineLengths[myLineNumber]++;
  printf("%s: chose SInspect %d with length %d\n", currentThread->getName(), myLineNumber, siLineLengths[myLineNumber]);

  waitingForSI_C[myLineNumber]->Signal(&siLineLock);
  waitingForSI_C[myLineNumber]->Wait(&siLineLock);
  
  siLineLengths[myLineNumber]--;
  siLineLock.Release();

  siLock[myLineNumber]->Acquire();

 // The Passenger now has the line number, so they should go to sleep and
  // release the line lock, letting another Passenger search for a line
  printf("%s giving airline ticket to Security Inspector %d\n", currentThread->getName(), myLineNumber);
  waitingForTicket_SI_C[myLineNumber]->Signal(siLock[myLineNumber]);
  waitingForTicket_SI_C[myLineNumber]->Wait(siLock[myLineNumber]);
  siLock[myLineNumber]->Release();
  printf("-----Number of Passengers chosen inspector: %d\n",pass_si_count);
  pass_si_count++;
  
}

void AirportSimulation() {

  Thread *t; // Create a thread pointer variable
  char *name;
  int i; 

  /*
   * Needs Airlines
   * Bags and weights
   *
   */

  printf("Starting Airport Simulation\n");

  // -------------------------------------------------
  // Initialize Condition Variables
  
  // waitingForAL condition variable
  for(i = 0; i < numberOfAL; i++) {
    name = new char [20];
    sprintf(name, "WFAL_C%d",i);
    waitingForAL_C[i] = new Condition(name);
    al_busy[i] = true;
  }

  // waitingForCIS condition variable
  for(i = 0; i < numberOfCIS; i++) {
    name = new char [20];
    sprintf(name,"WFCIS_C%d",i);
    waitingForCIS_C[i] = new Condition(name);
    cis_busy[i] = true;
  }

  // waitingForSO condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char [20];
    sprintf(name,"WFSO_C%d",i);
    waitingForSO_C[i] = new Condition(name);
    so_busy[i] = true;
  }

  // waitingForSI condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char [20];
    sprintf(name,"WFSI_C%d",i);
    waitingForSI_C[i] = new Condition(name);
    si_busy[i] = true;
  }

  // waitingForTicket_AL condition variable
  for(i = 0; i < numberOfAL; i++) {
    name = new char [20];
    sprintf(name, "WFTICKET_AL_C%d",i);
    waitingForTicket_AL_C[i] = new Condition(name);
  }
  
  // waitingForTicket_CIS_C condition variable
  for(i = 0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name,"CISTICKET_C%d",i);
    waitingForTicket_CIS_C[i] = new Condition(name);
  }

  // waitingForTicket_SO_C condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"SOTICKET_C%d",i);
    waitingForTicket_SO_C[i] = new Condition(name);
  }

  // waitingForTicket_SI_C condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"SITICKET_C%d",i);
    waitingForTicket_SI_C[i] = new Condition(name);
  }

  // onBreakCIS_C condition variable
  for(i = 0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name, "CISBREAK_C%d",i);
    onBreakCIS_C[i] = new Condition(name);
  }
   
  //--------------------------------------------------

  // -------------------------------------------------
  // Initialize Airport Liaison Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfAL; i++) {
    name = new char[20];
    sprintf(name,"alLock%d",i);
    alLock[i] = new Lock(name);
  }
  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize Check in Staff Line Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfAirlines; i++) {
    name = new char[20];
    sprintf(name,"cisLineLock%d",i);
    cisLineLock[i] = new Lock(name);
  }
  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize Airline check in staff Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name,"cisLock%d",i);
    cisLock[i] = new Lock(name);
  }
  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize Security Officer check in staff Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"soLock%d",i);
    soLock[i] = new Lock(name);
  }

  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"siLock%d",i);
    siLock[i] = new Lock(name);
  }
  // -------------------------------------------------
  // -------------------------------------------------
  // Initialize the Line Lengths 

  // Line length for Airport Liaison
  for( i = 0; i < numberOfAL; i++) {
    alLineLengths[i] = 0;
  }

  // Line length for Airline check in staff
  for( i = 0; i < numberOfCIS; i++) {
    cisLineLengths[i] = 0;
  }

  // Line length for Airline check in staff
  for( i = 0; i < numberOfSO; i++) {
    soLineLengths[i] = 0;
  }

  // Line length for Airline check in staff
  for( i = 0; i < numberOfSO; i++) {
    siLineLengths[i] = 0;
  }
  // -------------------------------------------------


  // Create the 20 passenger for our airport simulation
  printf("Creating Passengers\n");
  for( i=0; i < numberOfPassengers; i++) {
    // Create a ticket for the passenger
    pass_ticket_buffer[i].passenger_number = i;
    pass_ticket_buffer[i].flight_number = (i%3);
    pass_ticket_buffer[i].checkin_counter = -1;
    boarding_pass_buffer[i].passenger_number = i;
    boarding_pass_buffer[i].flight_number = (i%3);
    boarding_pass_buffer[i].seat_number = -1;

    name = new char [20]; 
    sprintf(name,"Passenger%d",i);
    //printf("Creating %s\n",name);
    t = new Thread(name); // Give the Passenger a name
    t->Fork((VoidFunctionPtr)Passenger,i);
  }

  // Create all the Airport Staff First

  // Create the Airport Liaison
  for(i = 0; i < numberOfAL; i++) {
    name = new char[20];
    sprintf(name, "AL%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)AirportLiaison,i);
  }

  // Create the Airline Check In Staff
  for(i=0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name, "Airline check-in-staff%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)CheckInStaff,i);
  }
  
  // Create the Security Officer Staff
  for(i=0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name, "SecurityOfficer%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)SecurityOfficer,i);
  }
  
  // Create the Airline Check In Staff
  
  for(i=0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name, "SecurityInspector%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)SecurityInspector,i);
  }
  
 
}


//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
  DEBUG('t', "Entering SimpleTest");
  /*
    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
  */
  // TestSuite();
}
