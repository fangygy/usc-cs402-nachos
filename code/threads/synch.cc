// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

Lock::Lock(char* debugName) { 

  // Initialization of private variables in synch.h file
  // Refer to synch.h for detailed notes on each private variable

  name = debugName;
  thread = NULL;

  FREE = true;
  BUSY = false;

  // Create the wait queues for waiting threads
  wait_queue  = new List;

}
Lock::~Lock() { 

  delete wait_queue; // De allocate memeory

}
bool Lock::isHeldByCurrentThread() {
  // If the current running thread is the same as the thread pointer
  // (the thread that owns the lock), then function returns true
  if(currentThread == thread)
    return true;
  else
    return false;
}
void Lock::Acquire() {
  // Disable the interrupts to make Acquire an atomic operation
  IntStatus old = interrupt->SetLevel(IntOff);

  /*
    - GENERAL ALGORITHM FOR ACQUIRE() -
    if("I am lock owner") {
       already own the lock 
       restore interrupts and return
    } 
    if("Lock Available") {
       thread becomes busy
       thread becomes the lock owner 
       therad pointer goes to current thread 
    } else { 
       add myself to the wait queue
       put myself to sleep
    }
  */

  // If currentThread is equal to thread, that means that currentThread
  // owns the lock. In this case, we don't need to do anything except
  // restore the interrupts and return
  if(isHeldByCurrentThread()) {
    DEBUG('e',"This thread owns this Lock already\n");
    interrupt->SetLevel(old);
    return;
  }
  
  // If the lock is free, then we assign the thread owner to the currentThread
  // Else, the thread must be added to the wait queue and then put to sleep
  if(FREE) {
    // printf("%s is acquiring the lock\n", currentThread->getName());

    DEBUG('e',"LOCK %s IS BEING ACQUIRED\n", name);
    BUSY   = true; // The lock state becomes busy
    thread = currentThread; // The owner of the lock is the currentThread
    FREE   = false; // The lock state is not free (busy)  

  } else { // If the lock is not free, then we must attach the currentThread to the wait queue 
    DEBUG('e',"Lock is not available\n");
    wait_queue->Append(currentThread);
    currentThread->Sleep();
  }

  // Restore interrupts
  interrupt->SetLevel(old);
}
void Lock::Release() {
  // Disable the interrupts to make Release an atomic operation
  IntStatus old = interrupt->SetLevel(IntOff);

  /*
    - GENERAL ALGORITHM FOR RELEASE -
    if("I am not the lock owner") {
       print error message using DEBUG('some character',"some String message");
       restore interrupts
       return 
    } 
    if("A thread is waiting") {
       remove a thread from the wait queue
       put the thread on the ready queue
       make this therad the lock owner
    } else {
    // there is no thread waiting 
       make the lock available 
       clear lock ownership
    }
  */

  if(!isHeldByCurrentThread()) { // If the currentThread does not own this lock
                                 // then it cannot release it. In that case, we will 
                                 // "ignore" this request and return. 
    DEBUG('e',"This thread does not own the lock\n");
    interrupt->SetLevel(old);
    return;
  }
  
  if(!wait_queue->IsEmpty()) {   // If the wait queue is not empty that means there
                                 // are threads waiting to use this lock
                                 // We have to remove a thread from the wait queue
                                 // wake it up, and set it as the new owner of the lock
    Thread *newthread = (Thread*)wait_queue->Remove();
    scheduler->ReadyToRun(newthread); // Waking up the thread (adding it to CPU Scheduler Ready Queue) 
    thread = newthread; // The thread removed from the wait queue is now the lock owner
  } else {                       // If the wait queue is empty
                                 // then there is no one to acquire this lock 
                                 // and therefore the lock will have no owner
    FREE   = true; 
    BUSY   = false;
    thread = NULL;
  }

  // Restore interrupts to allow context switching
  interrupt->SetLevel(old);
}

Condition::Condition(char* debugName) { 
  
  // Initialization of private variables in Condition Class
  // Refer to synch.h for further details 

  name       = debugName;
  wait_queue = new List;
  lock       = NULL;

}

Condition::~Condition() { 
  delete wait_queue; // De allocate memory
}

void Condition::Wait(Lock* conditionLock) { 

  // Disable interrupts
  IntStatus old = interrupt->SetLevel(IntOff);
  DEBUG('e',"Wait func called\n");

  lock = conditionLock; // Associate the lock with the conditionLock, so we can keep track 
                        // of which Condition Variable goes with which Lock (essentially)
  
  if(conditionLock == NULL) {   // If a NULL lock is passed through
                                // then we simply ignore it and return
    DEBUG('e',"Condition Lock is NULL\n");
    interrupt->SetLevel(old);
    return;
  }

  // If the conditionLock is not null, however, we want the conditionLock to be released
  // so that another thread can go and Acquire() the lock. This ensures that each thread 
  // gets a fair amount of time, and that no one thread can hog a lock. 

  conditionLock->Release();
  DEBUG('e',"Releasing lock\n");
  wait_queue->Append(currentThread); // add the currentThread to the wait queue, so that we can 
                                     // Signal() it later 
  currentThread->Sleep();            // Sleep the Thread
  conditionLock->Acquire();          // When the thread is woken up, it will reAcquire the lock

  // Restore interrupts
  interrupt->SetLevel(old);

}

void Condition::Signal(Lock* conditionLock) { 
  // Disable interrupts
  IntStatus old = interrupt->SetLevel(IntOff);

  if(wait_queue->IsEmpty()) {   // If the wait queue is empty, there is no one to Signal
    interrupt->SetLevel(old);
    return;
  }
  
  if(lock != conditionLock) {   // If the lock is not the conditionLock 
                                // then all the threads in the wait queue don't care
                                // since they only want a conditionLock that is the same as the 
                                // the lock
    DEBUG('e',"Condition Lock does not Equal Lock");
    interrupt->SetLevel(old);
    return;
  }

  // If we get down here, the lock is equal to the conditionLock, therefore we remove a thread 
  // that is waiting, and essentially wake them up.
  
  Thread *thread = (Thread *)wait_queue->Remove();
  scheduler->ReadyToRun(thread);

  if(wait_queue->IsEmpty()) {  // If the wait queue is empty, but the Lock is the same as the 
                               // the condition Lock, then no threads are associated with that lock
                               // so we can reassign the lcok pointer to null
    lock = NULL;
  }

  // Restore interrupts
  interrupt->SetLevel(old);
}

void Condition::Broadcast(Lock* conditionLock) {

  // Broadcast simply finds all the threads that are in the wait queue, and tells each one 
  // to wake up

  while(!wait_queue->IsEmpty()) {
    Signal(conditionLock);
  }
 
}
