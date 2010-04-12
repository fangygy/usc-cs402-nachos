//ServerCond.cc
#include "ServerCond.h"

ServerCond::ServerCond() {
  myCondID = -1;
  condLockID = -1;
}

ServerCond::ServerCond(int theCondID) {
  CondWaitQueueLock = new Lock("Wait Q Lock");
  condWaitQueueSize = 0;
  condLockID = -1;
}

void ServerCond::Wait(CondOwner theOwner) {
  
  CondWaitQueueLock->Acquire();

  int waitQIndex = condWaitQueueSize;

  //put caller on the wait queue
  condWaitQueue[waitQIndex].machineID = theOwner.machineID;
  condWaitQueue[waitQIndex].mailboxNum = theOwner.mailboxNum;
  //condWaitQueue[waitQIndex].lockID = theOwner.lockID;
  condLockID = theOwner.lockID;
  printf("WAIT[%d]: machine: %d, box: %d\n",waitQIndex,theOwner.machineID, theOwner.mailboxNum);
  printf("WAIT[%d]:(from queue) machine: %d, box: %d\n",waitQIndex,condWaitQueue[waitQIndex].machineID, condWaitQueue[waitQIndex].mailboxNum);
  condWaitQueueSize++;

  CondWaitQueueLock->Release();

}

ClientAddr ServerCond::Signal(CondOwner theOwner) {
  
  CondWaitQueueLock->Acquire();
  if(condWaitQueueSize > 0) {
    //wait queue is not empty

    ClientAddr clientToWake;
    clientToWake.machineID = condWaitQueue[0].machineID;
    clientToWake.mailboxNum = condWaitQueue[0].mailboxNum;

    printf("SIGNAL(from queue): machine: %d, box: %d\n",condWaitQueue[0].machineID, condWaitQueue[0].mailboxNum);

     printf("SIGNAL: machine: %d, box: %d\n",clientToWake.machineID, clientToWake.mailboxNum);
    //clientToWake.lockID = waitQueue[0].lockID;

    //move the rest of the wait queue up one slot
    for(int i = 1; i < condWaitQueueSize; i++) {
      condWaitQueue[i-1] = condWaitQueue[i];
    }
    condWaitQueueSize--;

    CondWaitQueueLock->Release();

    return clientToWake;

  } else {
    //wait queue is empty
    CondWaitQueueLock->Release();
    ClientAddr noOwner; 
    noOwner.machineID = -1;
    noOwner.mailboxNum = -1;
    //noOwner.lockID = -1;
    return noOwner;
  }

}

ClientAddr* ServerCond::Broadcast() {
  
  

  CondWaitQueueLock->Acquire();

  ClientAddr *clientsToWake = new ClientAddr[condWaitQueueSize];
  for(int i = 0; i < condWaitQueueSize; i++) {
    clientsToWake[i] = condWaitQueue[i];
  }
  
  CondWaitQueueLock->Release();

  return clientsToWake;

}

void ServerCond::SendMsg(CondOwner theOwner, int msg) {
  
  PacketHeader outPktHdr;
  MailHeader outMailHdr;
  char buffer[MaxMailSize];

  outPktHdr.to = theOwner.machineID;
  outMailHdr.to = theOwner.mailboxNum;

  //outMailHdr.from = 0;

  sprintf(buffer, "%d", msg);

  outMailHdr.length = strlen(buffer) + 1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);
  if(!success) {
    //sending response to client failed
    printf("The postOffice Send failed. Terminating Nachos\n");
    interrupt->Halt();
  } else {
    printf("Sent reponse to client\n");
  }

}
