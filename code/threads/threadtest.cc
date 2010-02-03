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

#include "copyright.h"
#include "system.h"
#include "functions.h"
#ifdef CHANGED
#include "synch.h"
#endif

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

// ---------------------------------------------------------------------
/*
 * PART ONE PART ONE PART ONE PART ONE PART ONE PART ONE PART ONE PART ONE 
 * RUN TEST SUITE
 *
 *
 *
*/
// ---------------------------------------------------------------------

// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("\nStarting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("\nStarting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("\nStarting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("\nStarting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("\nStarting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}

// ---------------------------------------------------------------------
/*
 * PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO 
 * RUN AIRPORT SIMULATIONS
 *
 *
 *
*/
// ---------------------------------------------------------------------

// Objects for Airport Liaison
Condition *waitingForAL_C[7];
Condition *waitingForTicket_AL_C[7];
Lock alLineLock("al_LL");
Lock *alLock[7];
int alLineLengths[7];
bool al_busy[7];

// Objects for Check In Staff
Condition *waitingForCIS_C[5];
Lock cisLineLock("cis_LL");
int cisLineLengths[5];



void AirportLiaison(int myNumber) {
  while(true) {
    alLineLock.Acquire();

    if(alLineLengths[myNumber]> 0) {
      waitingForAL_C[myNumber]->Signal(&alLineLock);
    } else {
      al_busy[myNumber] = false;
    }
    
    alLock[myNumber]->Acquire();
    alLineLock.Release();

    printf("%s telling Passenger to step up to counter\n",currentThread->getName());
    waitingForTicket_AL_C[myNumber]->Wait(alLock[myNumber]);    
  }
}

void Passenger(int myNumber) {
  
  // Passenger acquires the lock so they can search for shortest line amongst all lines
  alLineLock.Acquire();

  // Declare the variable for the Passenger's line number
  // We will reuse this variable for all 
  int myLineNumber;

  // Set the Passenger's Line number
  printf("%s: Searching for the shortest line\n", currentThread->getName());
  myLineNumber = findShortestLine(alLineLengths,7);
  
  if((alLineLengths[myLineNumber] > 0)||(al_busy[myLineNumber])) {
    alLineLengths[myLineNumber]++;
    printf("%s chose Liaison %d to with a line of length %d\n",currentThread->getName(),myLineNumber,alLineLengths[myLineNumber]);
    waitingForAL_C[myLineNumber]->Wait(&alLineLock);
  }

  alLineLock.Release();
  alLock[myLineNumber]->Acquire();

  // Releases the line lock so other Passengers can go search for shortest 
  // Airport Liaison Lines
  waitingForAL_C[myLineNumber]->Signal(&alLineLock);
  printf("%s: Going to see Liaison %d",currentThread->getName(),myLineNumber);
  // alLineLock.Release();

  // Passenger obtains lock to interact with Airport Liaison
  // alLock[myLineNumber]->Acquire();
  
  // Passenger waits for Airport Liaison to signal them to come to counter

  // waitingForTicket_AL_C[myLineNumber]->Wait(alLock[myLineNumber]);
  alLineLengths[myLineNumber]--;

  // Passenger is told to go to counter, and hands their ticket to Liaison
  printf("%s going to Liaison and giving them ticket\n", currentThread->getName());
  waitingForTicket_AL_C[myLineNumber]->Signal(alLock[myLineNumber]);
  waitingForTicket_AL_C[myLineNumber]->Wait(alLock[myLineNumber]);

  alLock[myLineNumber]->Release();

  // Acquire the Lock to the line
  // Only use one lock for all 5 lines, because only one Passenger at 
  // a time can be looking for the shortest line 
  // printf("Acquiring Lock...");
  //cisLineLock.Acquire();

  // Set the Passenger's line number
  //myLineNumber = findShortestLine(cisLineLengths, 5);

  // Increment the length of the Passenger's line by one, since the Passenger
  // is now in that line
  //cisLineLengths[myLineNumber]++;

  // The Passenger now has the line number, so they should go to sleep and
  // release the line lock, letting another Passenger search for a line
  //waitingForCIS_C[myLineNumber]->Wait(&cisLineLock);
}

void AirportSimulation() {

  Thread *t; // Create a thread pointer variable
  char *name;
  int i; 

  int numberOfAL  = 7;
  int numberOfCIS = 5;

  /*
   * Needs Airline
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
  }

  // waitingForAL condition variable
  for(i = 0; i < numberOfAL; i++) {
    name = new char [20];
    sprintf(name, "WFTICKET_AL_C%d",i);
    waitingForTicket_AL_C[i] = new Condition(name);
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
  // Initialize the Line Lengths 

  // Line length for Airport Liaison
  for( i = 0; i < numberOfAL; i++) {
    alLineLengths[i] = 0;
  }

  // Line length for check in staff
  for( i = 0; i < numberOfCIS; i++) {
    cisLineLengths[i] = 0;
  }
  // -------------------------------------------------


  // Create the 20 passenger for our airport simulation
  printf("Creating Passengers\n");
  for( i=0; i < 20; i++) {
    name = new char [20]; 
    sprintf(name,"Passenger%d",i);
    //printf("Creating %s\n",name);
    t = new Thread(name); // Give the Passenger a name
    t->Fork((VoidFunctionPtr)Passenger,i);
  }

  // Create all the Airport Staff First

  // printf("Creating als\n");
  // Create the Airport Liaison
  for(i = 0; i < numberOfAL; i++) {
    name = new char[20];
    sprintf(name, "AL%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)AirportLiaison,i);
  }

  for(i = 0; i < numberOfAL; i++) {
    // printf("%d",alLineLengths[i]);
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
    TestSuite();
}
