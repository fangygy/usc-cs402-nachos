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

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {}
Lock::~Lock() {
  // Implement Lock Class here
  // Lock Class should have several Condition Variables
  // As well, Lock Class should have a Queue of Waiting Threads
  
  *thread = NULL;

  FREE = true;
  BUSY = false;

  // Create the wait queues for waiting threads
  wait_queue  = new List;
  ready_queue = new List;

}
void Lock::Acquire() {
  // Disable the interrupts to make Acquire an atomic operation
  IntStatus old = interrupt->SetLevel(IntOff);

  /*
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
    interrupt->SetLevel(old);
    return;
  }
  
  // If the lock is free, then we assign the thread owner to the currentThread
  // Else, the thread must be added to the wait queue and then put to sleep
  if(FREE) {
    BUSY = true;
    thread = currentThread;
    FREE = false;
  } else {
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

  if(!isHeldByCurrentThread()) {
    DEBUG('e',"This thread does not own the lock");
    interrupt->SetLevel(old);
  }
  
  if(!wait_queue->IsEmpty()) {
    Thread *newthread = (Thread*)wait_queue->Remove();
    ready_queue->Append(newthread);
    thread = currentThread;
  } else {
    FREE   = true;
    BUSY   = false;
    thread = NULL;
  }

  // Restore interrupts to allow context switching
  interrupt->SetLevel(old);
}

Condition::Condition(char* debugName) { 
  
  wait_queue = new List;

}

Condition::~Condition() { }

void Condition::Wait(Lock* conditionLock) { 

  // Disable interrupts
  IntStatus old = interrupt->SetLevel(IntOff);

  lock = conditionLock;
  if(conditionLock == NULL) {
    DEBUG('f',"Condition Lock is NULL");
    interrupt->SetLevel(old);
    return;
  }
  wait_queue->Append(currentThread);
  conditionLock->Release();
  currentThread->Sleep();
  conditionLock->Acquire();

  // Restore interrupts
  interrupt->SetLevel(old);

}

void Condition::Signal(Lock* conditionLock) { 
  // Disable interrupts
  IntStatus old = interrupt->SetLevel(IntOff);
  if(wait_queue->IsEmpty()) {
    interrupt->SetLevel(old);
    return;
  }
  
  if(lock != conditionLock) {
    DEBUG('g',"Condition Lock does not Equal Lock");
    interrupt->SetLevel(old);
    return;
  }

  Thread *thread = (Thread *)wait_queue->Remove();
  ready_queue->Append(thread);

  if(wait_queue->IsEmpty()) {
    lock = NULL;
  }

  // Restore interrupts
  interrupt->SetLevel(old);
}

void Condition::Broadcast(Lock* conditionLock) { }
