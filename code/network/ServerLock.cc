//ServerLock.cc
#include "ServerLock.h"


ServerLock::ServerLock() {
  //state = FREE;
  // waitQueueSize = 0;
  myLockID = -1;
  //toBeDestroyed = false;
}

ServerLock::ServerLock(int theLockID) {
  
  state = FREE;
  waitQueueSize = 0;
  myLockID = theLockID;
  toBeDestroyed = false;

  WaitQueueLock = new Lock("Wait Queue Lock");

}

void ServerLock::Acquire(LockOwner theOwner) {

  WaitQueueLock->Acquire();
  
  if(waitQueueSize > 0) {
    //there is a wait queue for the lock, put info on queue
    waitQueue[waitQueueSize] = theOwner;
    waitQueueSize++;

  } else {
    //no one waiting for the lock
    
    if(state == FREE) {
      //no one else waiting for lock and lock is available
      //acquire the lock
      owner = theOwner;
      state = BUSY;
      //TODO - inform client that the lock was acquired
      SendMsg(owner, myLockID);

    } else {
      //no one else waiting for lock, but lock busy
      //put info on queue

      waitQueue[waitQueueSize] = theOwner;
      waitQueueSize++;

    }

  }

  //release WaitQueueLock
  WaitQueueLock->Release();


}

void ServerLock::Release(LockOwner theOwner) {
  
  /* don't check ownership?
  if(theOwner.machineID != owner.machineID || theOwner.mailboxNum != owner.mailboxNum) {
    //the caller does not currently own the lock
    //TODO - send error message or something?
    return;
  }
  */

  //reset the lock ownership
  owner.machineID = -1;
  owner.mailboxNum = -1;

  //check if anyone is waiting to use this lock
  WaitQueueLock->Acquire();
  if(waitQueueSize > 0) {
    //someone waiting for lock
    
    //take the 1st person from the wait queue and make it the lock owner
    owner = waitQueue[0];
    //move anyone else in the wait queue up one slot
    for(int i=1; i<waitQueueSize; i++) {
      waitQueue[i-1] = waitQueue[i];
    }
    waitQueueSize--;

    WaitQueueLock->Release();

    //send message to new owner of lock:
    SendMsg(owner, myLockID);

  } else {
    //no one waiting for lock
    WaitQueueLock->Release();
    state = FREE;
  }
  

}

void ServerLock::Destroy() {
  
  if(state != FREE) {
    // lock is in use, mark it for destruction
    toBeDestroyed = true;
  }

  
  
}


void ServerLock::SendMsg(LockOwner theOwner, int msg) {
  
  PacketHeader outPktHdr;
  MailHeader outMailHdr;
  char buffer[MaxMailSize];
  
  outPktHdr.to = theOwner.machineID;
  outMailHdr.to = theOwner.mailboxNum;

  //outMailHdr.from = 0;
  
  sprintf(buffer, "%d", msg);

  outMailHdr.length = strlen(buffer) + 1;

  printf("About to Send response to client at machine %d, box %d. response=%s.\n",outPktHdr.to, outMailHdr.to,buffer);

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);
  if(!success) {
    //sending response to client failed
    printf("The postOffice Send failed. Terminating Nachos\n");
    interrupt->Halt();
  } else {
    printf("Sent response to client at machine %d, box %d. response=%s.\n",outPktHdr.to, outMailHdr.to,buffer);
  }

}
