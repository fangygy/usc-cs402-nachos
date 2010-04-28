// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

struct TokenRequest {
  int tokenNum;
  int mailboxNum;
  int machineNum;
};

struct Member {
  int machineNum;
  int mailboxNum;
};

Member clients[1000];

enum RequestType { UNKNOWN, CREATE_LOCK, ACQUIRE, RELEASE, DESTROY_LOCK, CREATE_CONDITION, WAIT, SIGNAL, BROADCAST, REGISTER, CREATE_MV, GET, SET, TOKEN, REGISTER_RESPONSE };

RequestType getNetThreadRequestType(char* req);

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	{
	  result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	}	
      
      buf[n++] = *paddr;
      
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );
      while(!result)
	{
	  result = machine->WriteMem(vaddr,1,(int)(buf[n++]));
	}
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

/* New Syscalls
 * Acquire, Release, Wait, Signal, Broadcast
 *
 */

int CreateLock_Syscall(int vaddr) {
#ifndef NETWORK
  KernelLockTableLock->Acquire();
 //IS OK???
  // Return position in kernel structure array
  int size = 16;
  int max_chars = 20;

  //limit size of lock name
  if((size < 1) || (size > max_chars)) {
    DEBUG('q',"TOO MANY CHARS\n");
    return -1;
  }
  
  int addressSpaceSize = currentThread->space->NumPages() * PageSize;
  //in the clear to create the lock
  char *lockName = new char[size+1];
  lockName[size] = '\0';

  //make sure we aren't creating any part of the lock outside the alloted space
  if(vaddr < 0 || (vaddr+size) >= addressSpaceSize) {
    DEBUG('q',"OUT OF BOUNDS\n");
    return -1;
  }

  // Make sure the table is not full 
  if(nextLockIndex >= MAX_LOCKS) {
    //The table is full of locks 
    KernelLockTableLock->Release();
    DEBUG('q',"LOCK TABLE FULL\n");
    return -1;
  }

  copyin(vaddr,size+1,lockName);

  //initialize important vars
  osLocks[nextLockIndex].lock = new Lock(lockName);
  osLocks[nextLockIndex].as = currentThread->space;
  //initialize lock to not in use
  osLocks[nextLockIndex].usageCounter = 0;
  //no one wants to destroy the lock right now
  osLocks[nextLockIndex].toBeDestroyed = FALSE;
  //report number of locks to user first...
  //increment number of locks
  int lockindex;
  lockindex = nextLockIndex;
  nextLockIndex++;
  KernelLockTableLock->Release();
  return lockindex;//-1 to get current lock index?
  // this may prove to have some problems if
  // we are context switched out before nextLockIndex is
  // returned
#else
  
  stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  int size = 16;
  char *lockName = new char[size];
  copyin(vaddr,size,lockName);

  cout<<"lock name: "<<lockName<<endl;
  string lockNameStr = "L ";
  lockNameStr.append(lockName);
  cout<<"lockNameStr: "<<lockNameStr<<endl;

  strcpy(buffer, lockNameStr.c_str());

  //ss.clear();
  //ss<<lockNameStr;
  //ss<<"L "<<lockNameStr;
  //ss>>buffer;
  
  cout<<"buffer: "<<buffer<<endl;

  outPktHdr.to = postOffice->getNetAddr();
  /*  outMailHdr.to = 0;*/
  outMailHdr.to = currentThread->space->getMailbox();
  //printf("mailbox # (network thread): %d\n",currentThread->space->getMailbox());
  //printf("mailbox # (current thread): %d\n",currentThread->getMailbox());
  //printf("machine # (default): %d\n",outPktHdr.to);

  outMailHdr.from = currentThread->getMailbox();
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  postOffice->Receive(currentThread->getMailbox(), &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  fflush(stdout);
  
  ss.clear();
  ss.str(buffer);
  int lockID_rec;
  ss>>lockID_rec;


  //no one needs the token, send to the next group member
  //TODO - send the next group member (via the network thread)
	
  outPktHdr.to = postOffice->getNetAddr();
  outMailHdr.to = currentThread->space->getMailbox(); //send to network thread

  ss.clear();
  ss<<"T "<<lockID_rec;
  ss>>buffer;

  outMailHdr.length = strlen(buffer) + 1;
  success = postOffice->Send(outPktHdr, outMailHdr, buffer);
  if(!success) {
    printf("The post office send failed.\n");
    interrupt->Halt();
  }

  return lockID_rec; //will be -1 if lock table is full
  
  
  

#endif

}

void DestroyLock_Syscall(int value) {
#ifndef NETWORK
  // Delete from kernel structure array the lock object at position index
  int index = value;
  // Delete from kernel structure array the lock object at position index

  //First, acquire kernallocktablelock
  KernelLockTableLock->Acquire();
  
  //if the lock is in use, mark for destruction
  if(osLocks[index].usageCounter > 0){
    DEBUG('q',"CANNOT DESTROY LOCK IN USE\n");
    osLocks[index].toBeDestroyed = TRUE;
    KernelLockTableLock->Release();
    return;
  }
  
  //if the lock is already destined to be destroyed, return
  if((osLocks[index].usageCounter == 0) || (osLocks[index].toBeDestroyed == TRUE)){
    //destroy lock
    osLocks[index].toBeDestroyed = TRUE;
    //effectively destroy lock
    osLocks[index].lock = NULL;
    DEBUG('q',"DESTROYING LOCK\n");
    KernelLockTableLock->Release();
    return;
  }

#else

  stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  //int size = 16;
  //char *lockName = new char[size];
  //copyin(vaddr,size,lockName);

  //ss.clear();
  //ss<<"D "<<lockName;
  //ss>>buffer;

  sprintf(buffer, "D %d", value);
  
  outPktHdr.to = 0;
  outMailHdr.to = 0;
  outMailHdr.from = 1;
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%d\" from %d, box %d\n",buffer,inPktHdr.from,inPktHdr.from);
  fflush(stdout);

  ss.str(buffer);
  int lockID_rec;
  ss>>lockID_rec;

  if(lockID_rec < 0) {
    //there was an error destroying the lock
    DEBUG('q',"Error destroying lock\n");
  } else {
    //lock destroyed successfully (received confirmation from server)
    //do nothing
  }

#endif

}

void Acquire_Syscall(int index) {
#ifndef NETWORK
  KernelLockTableLock->Acquire();
  int value = index; // this is the value read by machine->readregister(4)
  DEBUG('a',"ACQUIRING LOCK\n\n\n\n\n");
  DEBUG('q',"lock index is: %d\n",index);
  // perform series of checks on the lock
  // to make sure user program is not doing
  // anything crazy

  //make sure the lock exists
  if(value < 0 || value >= nextLockIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }

  KernelLock curLock = osLocks[index];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('q',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  // The lock has not been destroyed
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space
    DEBUG('q',"%s : Acquire Syscall: LOCK %d BELONGS TO DIFFERENT PROCESS\n", currentThread->getName(), index);
    KernelLockTableLock->Release();
    return;
  }
  //ensure that lock isn't destroyed while in use
  curLock.usageCounter++; 
  DEBUG('q',"%s is ACQUIRING LOCK\n", currentThread->getName());
  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
    // FINALLY...Acquire the lock
  curLock.lock->Acquire();

#else

  stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  //int size = 16;
  //char *lockName = new char[size];
  //copyin(vaddr,size,lockName);

  //ss.clear();
  //ss<<"D "<<lockName;
  //ss>>buffer;

  sprintf(buffer, "A %d", index);
  
  outPktHdr.to = postOffice->getNetAddr();
  outMailHdr.to = currentThread->space->getMailbox();
  outMailHdr.from = currentThread->getMailbox();
  //outPktHdr.to = 0;
  //outMailHdr.to = 0;
  //outMailHdr.from = 1;
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  postOffice->Receive(currentThread->getMailbox(), &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%d\" from %d, box %d\n",buffer,inPktHdr.from,inPktHdr.from);
  fflush(stdout);

  ss.str(buffer);
  int lockID_rec;
  ss>>lockID_rec;

  //printf("ACQUIRE: Server Response:%d.\n",lockID_rec);

  if(lockID_rec < 0) {
    //there was an error acquiring the lock
    DEBUG('q',"Error acquiring lock\n");
  } else {
    //lock acquired successfully (received confirmation from server)
    //do nothing
  }

#endif

}

void Release_Syscall(int index) {
#ifndef NETWORK
  //NOTE: I just made this the same checks as Acquire for now....
  KernelLockTableLock->Acquire();
  int value = index;
  //make sure the lock exists
  if(value < 0 || value >= nextLockIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }
  KernelLock curLock = osLocks[index];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('q',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space    
    DEBUG('q',"Release Syscall: LOCK BELONGS TO DIFFERENT PROCESS\n");
    KernelLockTableLock->Release();
    return;
  }
  //ensure that lock isn't destroyed while in use
  DEBUG('q',"RELEASING LOCK\n");

  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
    // FINALLY...Release the lock
  curLock.lock->Release();
  curLock.usageCounter--;

#else
  
  stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];


  sprintf(buffer, "R %d", index);
  
  //outPktHdr.to = 0;
  //outMailHdr.to = 0;
  outPktHdr.to = postOffice->getNetAddr();
  outMailHdr.to = currentThread->space->getMailbox();
  outMailHdr.from = currentThread->getMailbox();
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  /*
  postOffice->Receive(currentThread->getMailbox(), &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%d\" from %d, box %d\n",buffer,inPktHdr.from,inPktHdr.from);
  fflush(stdout);

  ss.str(buffer);
  int lockID_rec;
  ss>>lockID_rec;

  if(lockID_rec < 0) {
    //there was an error releasing the lock
    DEBUG('q',"Error releasing lock\n");
  } else {
    //lock released successfully (received confirmation from server)
    //do nothing
  }
  */

#endif

}

int CreateCondition_Syscall() {
#ifndef NETWORK
  KernelCondTableLock->Acquire();
  //copied from CreateLock, IS OK???
  // Return position in kernel structure array
  int size = 16;
  int max_chars = 20;

  //limit size of condition name
  if((size < 1) || (size > max_chars)) {
    DEBUG('q',"TOO MANY CHARS\n");
    return -1;
  }
  
  int addressSpaceSize = currentThread->space->NumPages() * PageSize;
  //in the clear to create the condition
  char *condName = new char[size+1];
  condName[size] = '\0';

  //NO creating any part of the condition outside the alloted space
  /*if(vaddr < 0 || (vaddr+size) >= addressSpaceSize) {
    DEBUG('q',"OUT OF BOUNDS\n");
    return -1;
    }*/

  // Make sure the table is not full 
  if(nextCondIndex >= MAX_CONDS) {
    //The table is full of locks 
    KernelCondTableLock->Release();
    DEBUG('q',"COND TABLE FULL\n");
    return -1;
  }

  /*copyin(vaddr,size+1,condName);*/

  //initialize important vars
  osConds[nextCondIndex].condition = new Condition("");
  osConds[nextCondIndex].as = currentThread->space;
  //initialize condition to not in use
  osConds[nextCondIndex].usageCounter = 0;
  //no one wants to destroy the lock right now
  osConds[nextCondIndex].toBeDestroyed = FALSE;
  //report number of locks to user first...
  //increment number of locks
  int condindex = nextCondIndex;
  nextCondIndex++;
  KernelCondTableLock->Release();
  return condindex;//-1 to get current condition index?
  // this may prove to have some problems if
  // we are context switched out before nextCondIndex is
  // returned 

#else

  stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  //int size = 16;
  //char *cvName = new char[size];
  //copyin(vaddr,size,cvName);

  char *cvName = " ";

  ss.clear();
  ss<<"C "<<cvName;
  ss>>buffer;
  
  outPktHdr.to = 0;
  outMailHdr.to = 0;
  outMailHdr.from = 1;
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  fflush(stdout);
  
  ss.clear();
  ss.str(buffer);
  int condID_rec;
  ss>>condID_rec;

  return condID_rec; //will be -1 if condition table is full

#endif

}

void DestroyCondition_Syscall(int index) {
#ifndef NETWORK
  // Delete from kernel structure array the condition object at position index 
// Delete from kernel structure array the condition object at position index 
  //First, acquire kernalcondtablelock
  KernelCondTableLock->Acquire();
  
  //if the condition is in use, mark for destruction
  if(osConds[index].usageCounter > 0){
    DEBUG('q',"CANNOT DESTROY CONDITION IN USE\n");
    osLocks[index].toBeDestroyed = TRUE;
    KernelCondTableLock->Release();
    return;
  }
  
  //if the lock is already destined to be destroyed, return
  if((osConds[index].usageCounter == 0) || (osConds[index].toBeDestroyed == TRUE)){
    //destroy condition
    osConds[index].toBeDestroyed = TRUE;
    //effectively destroy lock
    osConds[index].condition = NULL;
    DEBUG('q',"DESTROYING CONDITION\n");
    KernelCondTableLock->Release();
    return;
  }
#else

  // do nothing?

#endif
}

void Wait_Syscall(int index, int lock_id) {
#ifndef NETWORK
  KernelCondTableLock->Acquire();
  //VALIDATE CONDITION
  //make sure the condition exists
  if(index < 0 || index >= nextCondIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }
  KernelCond curCond = osConds[index];
  if(curCond.condition == NULL) {
    // The condition has been destroyed 
    DEBUG('q',"COND HAS BEEN DESTROYED\n");
    return;
  }
  // The condition has not been destroyed
  if(curCond.as != currentThread->space) {
    // this condition belongs to a different process
    // since the address space of the condition does not match the 
    // current thread's address space
    DEBUG('q',"%s : Wait Syscall: CONDITION %d BELONGS TO DIFFERENT PROCESS\n", currentThread->getName(), index);
    KernelCondTableLock->Release();
    return;
  }
  //ensure that condition isn't destroyed while in use
  curCond.usageCounter++; 
  KernelCondTableLock->Release();
  KernelLockTableLock->Acquire();
  //VALIDATE LOCK
  //make sure the lock exists
  if(lock_id < 0 || lock_id >= nextLockIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }

  KernelLock curLock = osLocks[lock_id];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('q',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  // The lock has not been destroyed
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space
    DEBUG('q',"%s : Wait Syscall: LOCK %d BELONGS TO DIFFERENT PROCESS\n", currentThread->getName(), lock_id);
    KernelLockTableLock->Release();
    return;
  }
  //ensure that lock isn't destroyed while in use
  curLock.usageCounter++;
  DEBUG('q',"CONDITION WAITING\n");
  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
  // FINALLY...use wait on the lock
  curCond.condition->Wait(curLock.lock);

#else

  //stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  //int size = 16;
  //char *cvName = new char[size];
  //copyin(vaddr,size,cvName);

  //ss.clear();
  //ss<<"C "<<cvName;
  //ss>>buffer;

  sprintf(buffer, "W %d %d", index, lock_id);
  
  outPktHdr.to = 0;
  outMailHdr.to = 0;
  outMailHdr.from = 1;
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }



  postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  fflush(stdout);
 

#endif
}

void Signal_Syscall(int index, int lock_id) {
#ifndef NETWORK
  KernelCondTableLock->Acquire();
  //VALIDATE CONDITION
  //make sure the condition exists
  if(index < 0 || index >= nextCondIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }
  KernelCond curCond = osConds[index];
  if(curCond.condition == NULL) {
    // The condition has been destroyed 
    DEBUG('q',"COND HAS BEEN DESTROYED\n");
    return;
  }
  // The condition has not been destroyed
  if(curCond.as != currentThread->space) {
    // this condition belongs to a different process
    // since the address space of the condition does not match the 
    // current thread's address space
    DEBUG('q',"%s: Signal Syscall: CONDITION %d BELONGS TO DIFFERENT PROCESS\n", currentThread->getName(), index);
    KernelCondTableLock->Release();
    return;
  }
  //ensure that condition isn't destroyed while in use
  curCond.usageCounter--; 
  KernelCondTableLock->Release();
  KernelLockTableLock->Acquire();
  //VALIDATE LOCK
  //make sure the lock exists
  if(lock_id < 0 || lock_id >= nextLockIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }

  KernelLock curLock = osLocks[lock_id];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('q',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  // The lock has not been destroyed
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space
    DEBUG('q',"%s: Signal Syscall: LOCK %d BELONGS TO DIFFERENT PROCESS\n", currentThread->getName(), index);
    KernelLockTableLock->Release();
    return;
  }
  //ensure that lock isn't destroyed while in use
  curLock.usageCounter--;
  DEBUG('q',"CONDITION SIGNAL\n");
  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
  // FINALLY...use signal on the lock
  curCond.condition->Signal(curLock.lock);

#else

  //stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  //int size = 16;
  //char *cvName = new char[size];
  //copyin(vaddr,size,cvName);

  //ss.clear();
  //ss<<"C "<<cvName;
  //ss>>buffer;

  sprintf(buffer, "S %d %d", index, lock_id);
  
  outPktHdr.to = 0;
  outMailHdr.to = 0;
  outMailHdr.from = 1;
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  // Signal does not expect any response message

  //postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
  //printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  //fflush(stdout);

#endif

}

void Broadcast_Syscall(int index, int lock_id) {
#ifndef NETWORK
  KernelCondTableLock->Acquire();
  //VALIDATE CONDITION
  //make sure the condition exists
  if(index < 0 || index >= nextCondIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }
  KernelCond curCond = osConds[index];
  if(curCond.condition == NULL) {
    // The condition has been destroyed 
    DEBUG('q',"COND HAS BEEN DESTROYED\n");
    return;
  }
  // The condition has not been destroyed
  if(curCond.as != currentThread->space) {
    // this condition belongs to a different process
    // since the address space of the condition does not match the 
    // current thread's address space
    DEBUG('q',"Broadcast Syscall: CONDITION BELONGS TO DIFFERENT PROCESS\n");
    KernelCondTableLock->Release();
    return;
  }
  //ensure that condition isn't destroyed while in use
  int temp = curCond.usageCounter;
  curCond.usageCounter--; 
  KernelCondTableLock->Release();
  KernelLockTableLock->Acquire();
  //VALIDATE LOCK
  //make sure the lock exists
  if(lock_id < 0 || lock_id >= nextLockIndex) {
    // This is a bad value
    DEBUG('q',"BAD VALUE\n");
    return;
  }

  KernelLock curLock = osLocks[lock_id];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('q',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  // The lock has not been destroyed
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space
    DEBUG('q',"Broadcast Syscall: LOCK %d BELONGS TO DIFFERENT PROCESS\n",lock_id);
    KernelLockTableLock->Release();
    return;
  }
  //ensure that lock isn't destroyed while in use
  if(curLock.usageCounter - temp >= 0){
    curLock.usageCounter = curLock.usageCounter - temp;
  }else{
    curLock.usageCounter = 0;
  }
  DEBUG('q',"CONDITION BROADCAST\n");
  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
  // FINALLY...use broadcast on the lock
  curCond.condition->Broadcast(curLock.lock);


#else

  //stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];

  //int size = 16;
  //char *cvName = new char[size];
  //copyin(vaddr,size,cvName);

  //ss.clear();
  //ss<<"C "<<cvName;
  //ss>>buffer;

  sprintf(buffer, "B %d %d", index, lock_id);
  
  outPktHdr.to = 0;
  outMailHdr.to = 0;
  outMailHdr.from = 1;
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);

  if(!success) {
    printf("The postOffice Send failed.\n");
    interrupt->Halt();
  }

  // Broadcast does not expect any response message

  //postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
  //printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  //fflush(stdout);

#endif

}

void Yield_Syscall() {
  currentThread->Yield();
}

void Exit_Syscall(int status) {
  //TODO - finish this
  //interrupt->Halt();

  //cout<<"id of current thread= "<<currentThread->getMyId()<<endl;

  currentThread->Finish();
 
  int i, spaceId_f;
  // get the space id for this new thread
  /*
  for(i=0; i<64; i++) {
    if(processTable[i].as == currentThread->space) {
      spaceId_f = i;
      break;
    } else { 
      // Trying to exit a thread without an existing address space
      DEBUG('a', "Trying to exit a thread without an existing address space\n");
      interrupt->Halt();
    }
  }
  */
  //cout<<"num child processes: "<<processTable[spaceId_f].numChildProcess<<endl;
  if(processTable[spaceId_f].numChildProcess > 0) {
    // do something
  }

}

void Register_Syscall() {
  stringstream ss;

  PacketHeader outPktHdr, inPktHdr;
  MailHeader outMailHdr, inMailHdr;
  char buffer[MaxMailSize];
  //char *data = "J";

  ss.clear();
  ss << "J";
  ss >> buffer;

  outPktHdr.to = postOffice->getNetAddr();
  outMailHdr.to = currentThread->space->getMailbox();
  outMailHdr.from = currentThread->getMailbox();
  outMailHdr.length = strlen(buffer)+1;

  bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);
  if(!success) {
    printf("The post office send failed.\n");
    interrupt->Halt();
  }
  
  postOffice->Receive(currentThread->getMailbox(), &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);

  fflush(stdout);
  /*
  ss.clear();
  ss.str(buffer);

  string currentClient;
  int numClients = 0;
  //Member clients[1000];
  printf("Member List Received:\n");
  while( getline(ss, currentClient, ' ')) {
    
    int firstCommaPos = currentClient.find_first_of(",");
    //cout<<"firstCommaPos:"<<firstCommaPos<<endl;
    //int lastPeriodPos = currentClient.find_last_of(".");
    string machineNumStr = currentClient.substr(0,firstCommaPos);
    string mailboxNumStr = currentClient.substr(firstCommaPos+1);
    //cout<<"machine num: "<<machineNumStr<<" mailbox num: "<<mailboxNumStr<<endl;
    int machineNum = atoi(machineNumStr.c_str());
    int mailboxNum = atoi(mailboxNumStr.c_str());
    clients[numClients].machineNum = machineNum;
    clients[numClients].mailboxNum = mailboxNum;
    numClients++;
    printf("   Machine: %d, Mailbox: %d\n", machineNum, mailboxNum);
    //cout<<"machine num: "<<machineNum<<" mailbox num: "<<mailboxNum<<endl;
  }
  printf("There are %d clients (including myself)\n",numClients);
  */
}

/* Returns the value of the monitor variable at the index specified */
int GetMV_Syscall(int index) {
  int value;
  return value;
}

/* This syscall will talk to the networking thread and tell it to tell the server */
/* to change a value of a monitor variable */
void SetMV_Syscall(int index, int value) {

}

int CreateMV_Syscall(unsigned int vaddr) {
  return 0; // for now
}

void netThread() {
  
  int numClients = 0;
  Member clients[1000]; //other Network Threads


  int myMailboxNum = currentThread->getMailbox(); //currentThread->mailbox;
  
  Lock *AcquireQueueLock = new Lock("acquireQueueLock");
  int acquireQueueLength = 0;
  TokenRequest acquireQueue[1000];
  TokenRequest myTokens[1000]; //keeps track of the tokens acquired by this network thread's user programs
  int myTokensLength = 0;
  Member nextClient;
  int myClientID = -1;

  Member myUserProgs[1000];
  int numUserProgs = 0;
  bool registered = false; //set to true when this network thread (and all the other members) register with the server
  Lock* MyUserProgsLock = new Lock("NT User Progs Lock");
  
  while(true) {
    // get the message 
    // determine what to do with the message
    stringstream ss;

    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char* param = new char;
    char* param2 = new char; //second param, if needed
    char* request = new char;
    char* response = new char;

    int tokenID = -1;

    string currentClientResp;
    
    int numClientsResp = 0;

    
    //outPktHdr.to = 0;
    //outMailHdr.to = 0;
    outMailHdr.from = myMailboxNum;
    //outMailHdr.length = strlen(buffer)+1;
    
    //bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);
    
    //if(!success) {
    //printf("The postOffice Send failed.\n");
    //interrupt->Halt();
    //}
    
    postOffice->Receive(myMailboxNum, &inPktHdr, &inMailHdr, buffer); 
    printf("Network Thread: Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inPktHdr.from);
    fflush(stdout);
    
    ss.str(buffer);
    ss >> request;
    ss >> param;

    printf("NT: request=\"%s\"\n",request);

    RequestType r = getNetThreadRequestType(request);

    int myClientIndex = -1;
    string clientListStr = "";
    string nextWord = "";

    bool canReleaseToken = false;
    int releaseMyTokenIndex = -1;
    
    switch(r) {
    case CREATE_LOCK:
      // If this is a create lock
      // send message to server

      ss.clear();
      ss<<"C "<<param;
      ss>>buffer;

      int fromMailbox = inMailHdr.from;
      int fromMachine = inPktHdr.from;
      outPktHdr.to = 0;
      outMailHdr.to = 0;
      outMailHdr.length = strlen(buffer) + 1;
      bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);
      if(!success) {
	printf("The post office send failed.\n");
	interrupt->Halt();
      }
      // Sends out the token id for every new message, even with OK message     
      postOffice->Receive(myMailboxNum, &inPktHdr, &inMailHdr, buffer);
      printf("Network Thread: Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
      fflush(stdout);
      // Parse the message, and the token id
      // which will be an integer
      ss.clear();
      ss.str(buffer);
      int lockID_rec;
      ss>>lockID_rec;

      //string nextWord;
      ss>>nextWord;
      if(nextWord == "dup") {
	//this lock already existed, it is not new
	//do not send the token around (it is not my token)

	printf("Network Thread: The token (id=%d) already existed.\n",lockID_rec);
	
      } else {

	printf("Network Thread: Received (new) token ID %d\n",lockID_rec);

	//send the token to my neighbor (start the cycle)
      
	string tokenStr = "T ";
	string tokenNumStr ="";
	
	ss.clear();
	ss<<lockID_rec;
	tokenNumStr = ss.str();
	//ss>>tokenNumStr;
	//cout<<"tokenNumStr:"<<tokenNumStr<<endl;

	tokenStr.append(tokenNumStr);
	strcpy(buffer,tokenStr.c_str());
	//ss>>buffer;

	//cout<<"buffer: "<<buffer<<endl;
	outPktHdr.to = nextClient.machineNum;
	outMailHdr.to = nextClient.mailboxNum;
	outMailHdr.length = strlen(buffer) + 1;
	success = postOffice->Send(outPktHdr, outMailHdr, buffer);
	if(!success) {
	  printf("The post office send failed.\n");
	  interrupt->Halt();
	}

      }

      //send the token number to the caller
      ss.clear();
      ss<<lockID_rec;
      ss>>buffer;
      outPktHdr.to = fromMachine;
      outMailHdr.to = fromMailbox;
      outMailHdr.length = strlen(buffer) + 1;
      success = postOffice->Send(outPktHdr, outMailHdr, buffer);
      if(!success) {
	printf("The post office send failed.\n");
	interrupt->Halt();
      }

      break;
    case ACQUIRE:
      // Acquire a lock
      ss>>param2;
      ss.clear();
      //ss.str(param2);
      //ss>>tokenID;
      tokenID = atoi(param2);
      

      //cout<<"NT: UP wants to acquire token id="<<tokenID<<endl;
      AcquireQueueLock->Acquire();
      acquireQueue[acquireQueueLength].machineNum = inPktHdr.from;
      acquireQueue[acquireQueueLength].mailboxNum = inMailHdr.from;
      acquireQueue[acquireQueueLength].tokenNum = tokenID;
      acquireQueueLength++;
      AcquireQueueLock->Release();
      
      
      break;
    case RELEASE:
      ss>>param2;
      ss.clear();
      //ss.str(param2);
      //ss>>tokenID;
      tokenID = atoi(param2);
      

      //cout<<"NT: UP wants to release token id="<<tokenID<<endl;

      for(int i = 0; i < myTokensLength; i++) {
	//if(myTokens[i].tokenNum == tokenID && myTokens[i].machineNum == inPktHdr.from && myTokens[i].mailboxNum == inMailHdr.from) {
	if(myTokens[i].tokenNum == tokenID) {
	  //The release request came from the same person who requested the token
	  canReleaseToken = true;
	  releaseMyTokenIndex = i;
	  break;
	}
      }

      if(canReleaseToken) {
	//The release request came from the same person who requested the token
	for(int i=releaseMyTokenIndex; i < myTokensLength-1; i++) {
	  myTokens[i] = myTokens[i+1];
	}
	myTokensLength--;

	//ss.clear();
	//ss << "T "<< tokenID;
	//ss >> buffer;

	sprintf(buffer, "T %d", tokenID);
       
	outPktHdr.to = nextClient.machineNum;
	outMailHdr.to = nextClient.mailboxNum;
	outMailHdr.from = myMailboxNum;
	outMailHdr.length = strlen(buffer)+1;

	//printf("NT: Sending token %d (after release) to %d,%d\n",tokenID,outPktHdr.to,outMailHdr.to);
	//printf("NT: buffer: %s\n",buffer);
       
	bool tokenSuccess = postOffice->Send(outPktHdr, outMailHdr, buffer);
	if(!tokenSuccess) {
	  printf("The post office send failed.\n");
	  interrupt->Halt();
	}

      } else {
	//the user doesn't have permission to release this lock
	printf("Network Thread: The user does not have permission to release this token.\n");
      }



      // On release, just send the token to the next UP
      /*
      AcquireQueueLock->Acquire();
      if(acquireQueueLength == 0) {
         //send token to the next UP
      } else {
         //send msg to the first thread waiting

	 outPktHdr.to = acquireQueue[0].machineNum;
	 outMailHdr.from = acquireQueue[0].mailboxNum;
	 
	 for(int i=1; i < acquireQueueLength; i++) {
	    acquireQueue[i-1] = acquireQueue[i];
	 }
	 acquireQueueLength--;
	 AcquireQueueLock->Release();

	 //TODO- send token # to the 1st thread waiting
	 postOffice->Send();
      }
      */
      break;
    case REGISTER:

       ss.clear();
       ss << "J";
       ss >> buffer;

       MyUserProgsLock->Acquire();
       myUserProgs[numUserProgs].machineNum = inPktHdr.from;
       myUserProgs[numUserProgs].mailboxNum = inMailHdr.from;
       numUserProgs++;
       MyUserProgsLock->Release();
       
       outPktHdr.to = 0;
       outMailHdr.to = 0;
       outMailHdr.from = myMailboxNum;
       outMailHdr.length = strlen(buffer)+1;
       
       bool regSuccess = postOffice->Send(outPktHdr, outMailHdr, buffer);
       if(!regSuccess) {
	 printf("The post office send failed.\n");
	 interrupt->Halt();
       }

       

       break;
       
    case REGISTER_RESPONSE:
      ss.clear();

      ss.str(buffer);

      //string currentClientResp;
      currentClientResp = "";
      numClientsResp = 0;

      //Member clients[1000];
      printf("Member List Received:\n");
      //int myClientIndex = -1;
      while( getline(ss, currentClientResp, ' ')) {
	//printf("starting\n");
	//cout<<"\""<<currentClientResp<<"\""<<endl;
	string registerResponseMsgTitle = "X";
	if(currentClientResp == registerResponseMsgTitle) {
	  continue;
	}
	//printf("currentClientResp=%s\n",currentClientResp);
	int firstCommaPos = currentClientResp.find_first_of(",");
	string machineNumStr = currentClientResp.substr(0,firstCommaPos);
	string mailboxNumStr = currentClientResp.substr(firstCommaPos+1);
	int machineNum = atoi(machineNumStr.c_str());
	int mailboxNum = atoi(mailboxNumStr.c_str());
	if(machineNum == postOffice->getNetAddr() && mailboxNum == myMailboxNum) {
	  myClientID = numClientsResp;
	}
	clients[numClientsResp].machineNum = machineNum;
	clients[numClientsResp].mailboxNum = mailboxNum;
	numClientsResp++;
	printf("   Machine: %d, Mailbox: %d\n", machineNum, mailboxNum);
	//cout<<"machine num: "<<machineNum<<" mailbox num: "<<mailboxNum<<endl;
      }
      
      if(myClientID < 0) {
	//my client ID was not in the list of registered clients... problem
	printf("This network thread was not in the list of registered clients\n");
	interrupt->Halt();
      } else {
	if(myClientID == numClientsResp - 1) {
	  //I am the last client in the list.
	  //my next client is the 1st client in the list
	  nextClient.machineNum = clients[0].machineNum;
	  nextClient.mailboxNum = clients[0].mailboxNum;
	} else {
	  if(numClientsResp > 1) {
	    //I am not the only client in the list of registered clients
	    nextClient.machineNum = clients[myClientID+1].machineNum;
	    nextClient.mailboxNum = clients[myClientID+1].mailboxNum;
	  } else {
	    //I am the only client in the list of registered clients
	    //(this case should be covered above (will never reach here)
	  }
	}
      }

      printf("Network Thread: There are %d clients (including myself)\n",numClientsResp);
      printf("Network Thread: My machine/mailbox is %d,%d. My neighbor is at %d,%d.\n",postOffice->getNetAddr(),myMailboxNum,nextClient.machineNum,nextClient.mailboxNum);

      registered = true;
      MyUserProgsLock->Acquire();
      ss.clear();
      ss << "REG";
      ss >> buffer;
      for(int i =0; i<numUserProgs; i++) {
	outPktHdr.to = myUserProgs[i].machineNum;
	outMailHdr.to = myUserProgs[i].mailboxNum;
	outMailHdr.length = strlen(buffer)+1;
	bool rrSuccess = postOffice->Send(outPktHdr, outMailHdr, buffer);
	if(!rrSuccess) {
	  printf("The post office send failed.\n");
	  interrupt->Halt();
	}
	
      }
      MyUserProgsLock->Release();

      break;
       
    case WAIT:
      break;

    case TOKEN:
      {
      ss>>param2;
      ss.clear();
      //ss.str(param2);
      //ss>>tokenID;

      tokenID = atoi(param2);

      printf("Network Thread: Received token id=%d\n",tokenID);
     
      bool tokenNeeded = false;

      AcquireQueueLock->Acquire();
      //printf("Network Thread: acquire queue length currently=%d\n",acquireQueueLength);
      for(int i=0; i < acquireQueueLength; i++) {
	if(acquireQueue[i].tokenNum == tokenID) {
	  //there is a thread waiting for this token
	  tokenNeeded = true;
	  outPktHdr.to = acquireQueue[i].machineNum;
	  outMailHdr.to = acquireQueue[i].mailboxNum;

	  myTokens[myTokensLength].machineNum = acquireQueue[i].machineNum;
	  myTokens[myTokensLength].mailboxNum = acquireQueue[i].mailboxNum;
	  myTokensLength++;

	  printf("My user program wants to acquire token id=%d\n",tokenID);

	  for(int j = i+1; j < acquireQueueLength; j++) {
	    acquireQueue[j-1] = acquireQueue[j];
	  }
	  acquireQueueLength--;

	  ss.clear();
	  ss<<tokenID;
	  ss>>buffer;
	  outMailHdr.length = strlen(buffer) + 1;
	  success = postOffice->Send(outPktHdr, outMailHdr, buffer);
	  if(!success) {
	    printf("The post office send failed.\n");
	    interrupt->Halt();
	  }

	}
	break;
      }
      AcquireQueueLock->Release();

      if(!tokenNeeded) {
	//no one needs the token, send to the next group member
	//TODO - send the next group member

	int tempWait = 0;
	
	while(tempWait < 10000) {
	  currentThread->Yield();
	  tempWait++;
	}
	
	outPktHdr.to = nextClient.machineNum;
	outMailHdr.to = nextClient.mailboxNum; //TODO - FIX THIS

	//buffer remains the same as it was received
	outMailHdr.length = strlen(buffer) + 1;
	success = postOffice->Send(outPktHdr, outMailHdr, buffer);
	if(!success) {
	  printf("The post office send failed.\n");
	  interrupt->Halt();
	}

      }
      } // end case bracket
      

      break;

    case GET:

      break;
    case SET:
      break;
    case CREATE_MV:
      break;
      
      //case 'SignalReply':
      //break;
      //case 'RegistrationReply':
      //break;
    }
  }
  
}

void execThread() {
  DEBUG('f',"running the thread %d\n", currentThread->space->id);
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  machine->Run();
  DEBUG('f',"running the thread\n");
}

void kernelFunc(int vaddr) {
  int i, spaceId;  
  IntStatus old = interrupt->SetLevel(IntOff);
  spaceId = currentThread->space->id;
  // write to register PCReg the virtual address
  machine->WriteRegister(PCReg, vaddr);
  
  // write virtual address + 4 in NextPCReg
  machine->WriteRegister(NextPCReg, (vaddr+4));
  // call RestoreState function
  // restore state sets the machine->pageTable to the current address space's page table
  currentThread->space->RestoreState();

  // write to stack register, the starting position of the stack
  DEBUG('g', "kernel func: space id: %d \n",spaceId);
  DEBUG('h',"current Thread space size %d",currentThread->space->NumPages());

  machine->WriteRegister(StackReg,currentThread->stackLoc);
  //printf("stack location: %d\n", processTable[spaceId].stackLocation);
  interrupt->SetLevel(old);
  machine->Run();
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv = 0;
    
    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
	    case SC_CreateLock:
	        DEBUG('a', "CreateLock syscall.\n");
	        rv = CreateLock_Syscall(machine->ReadRegister(4));
		break;
	    case SC_DestroyLock:
	        DEBUG('a', "DestroyLock syscall.\n");
	        DestroyLock_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Acquire:
		DEBUG('a', "Close syscall.\n");
		Acquire_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Release:
		DEBUG('a', "Release syscall.\n");
		Release_Syscall(machine->ReadRegister(4));
		break;
	    case SC_CreateCondition:
	        DEBUG('a', "CreateCondition syscall.\n");
	        rv = CreateCondition_Syscall();
		break;
	    case SC_DestroyCondition:
	        DEBUG('a', "DestroyCondition syscall.\n");
	        DestroyCondition_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Wait:
		DEBUG('a', "Wait syscall.\n");
		Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Signal:
		DEBUG('a', "Signal syscall.\n");
		Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Broadcast:
		DEBUG('a', "Broadcast syscall.\n");
		Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Yield:
	        DEBUG('a', "Yield syscall. \n");
		Yield_Syscall();
		break;
	    case SC_Exit:
		int spaceid_ex;
		spaceid_ex = currentThread->space->id;
		for(int i = 0; i < TLBSize; i++) {
		  // invalidate the tlb 
		  IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable Interrupts
		  // machine->tlb[i].valid = FALSE;
		  interrupt->SetLevel(oldLevel); // Disable Interrupts
		}
		currentThread->Finish();
		break;
	    case SC_Fork:
	        DEBUG('a', "Fork syscall.\n");
		// get the virtual address of function being forked
		int virtualAddress;
		int name;
		virtualAddress = machine->ReadRegister(4);
		name = machine->ReadRegister(5);
		Thread *kernelThread;
		int i, spaceId_f;
		// get the space id for this new thread
		spaceId_f = currentThread->space->id;
		DEBUG('d',"space id: %d\n",spaceId_f);
		char *buff = new char[16+1];	// Kernel buffer to put the name in
		if( copyin(virtualAddress,16,buff) == -1 ) {
		  DEBUG('a',"Bad pointer passed to Open\n");
		  delete[] buff;
		}
		DEBUG('g',"fork: space id : %d\n", spaceId_f);
		if(name == 1) {
		  kernelThread = new Thread("Passenger");
		} else if(name == 2) {
		  kernelThread = new Thread("AirportLiaison");
		} else if(name == 3) {
		  kernelThread = new Thread("CheckInStaff");
		} else if(name == 4) {
		  kernelThread = new Thread("SecurityOfficer");
		} else if(name == 5) {
		  kernelThread = new Thread("SecurityInspector");
		} else if(name == 6) {
		  kernelThread = new Thread("CargoHandler");
		} else if(name == 7) {
		  kernelThread = new Thread("AirportManager");
		} else {
		  kernelThread = new Thread("kernelThread");
		}
		// this is the same as the currentThread->space b/c this thread
		// is a child of the currentThread
		//printf("address space num pages %d \n", currentThread->space->NumPages());
		kernelThread->space = currentThread->space;

		// Create a new page table with 8 pages more of stack
		kernelThread->space->NewPageTable();
		kernelThread->stackLoc = (kernelThread->space->NumPages()*PageSize)-16;
		/*
		if(processTable[spaceId_f].as == currentThread->space) {
		  //printf("process table address space pointer is equal to current thread\n");
		}
		*/
		// Update process table
		// DEBUG('g',"address space address: %d \n",&(processTable[spaceId_f].as));
		//processTable[spaceId_f].stackLocation = (kernelThread->space->NumPages()*PageSize)-16;
		//printf("address space num pages %d \n", currentThread->space->NumPages());

		//printf("stack location: %d\n", processTable[spaceId_f].stackLocation);
		// processTable[spaceId_f].numChildProcess++;
		kernelThread->Fork(kernelFunc,virtualAddress);
		
		break;
	    case SC_Exec:
	        DEBUG('d',"Exec syscall. \n");
		
		int virtualAddress_e, physicalAddress_e; 
		char* filename;
		// Get the virtual address for the name of the process
		// virtualAddress_e = machine->ReadRegister(4);
		virtualAddress_e = machine->ReadRegister(4);
		char *buf = new char[32+1];	// Kernel buffer to put the name in
		OpenFile *f;			// The new open file
		int id;				// The openfile id
		
		if (!buf) {
		  DEBUG('d',"Can't allocate kernel buffer in Open\n");
		 
		}
		
		if( copyin(virtualAddress_e,32,buf) == -1 ) {
		  DEBUG('d',"Bad pointer passed to Open\n");
		  delete[] buf;
		}
		
		buf[32]='\0';

		// Print out filename
		printf("Opening %s\n",buf);

		f = fileSystem->Open(buf);
		if(f == NULL) {
		  printf("%s","Unable to open file\n");
		  
		} else {

		  AddrSpace *space;
		  DEBUG('d',"Got the file open\n");
		  
		  // create a new address space for this executable file
		  space = new AddrSpace(f);
		  Thread *executionThread = new Thread("executionThread");
		  // Allocate the address space to this thread
		  
		  mailboxLock->Acquire();
		  executionThread->setMailbox(nextMailbox);
		  nextMailbox++;
		  Thread *networkThread = new Thread("networkThread");
		  networkThread->setMailbox(nextMailbox);
                  space->setMailbox(nextMailbox);
		  nextMailbox++;
		  mailboxLock->Release();
		  
		  DEBUG('d',"allocated the address space\n");
		  numProcesses++;
		  space->id = numProcesses;
		  executionThread->space = space;
 
		  DEBUG('g',"space id: %d \n",executionThread->space->id);

		  // Write the space id to register 2
		  rv = space->id;
		  // Fork the thread
		  executionThread->Fork((VoidFunctionPtr)execThread,0);
		  networkThread->Fork((VoidFunctionPtr)netThread,0);
		}
		break;
	case SC_Print:
	  int virtualAddress_p;
	  int p1, p2, p3;
	  char *buf_p = new char[168+1];
	  
	  virtualAddress_p = machine->ReadRegister(4);
	  p1 = machine->ReadRegister(5);
	  p2 = machine->ReadRegister(6);
	  p3 = machine->ReadRegister(7);
	  if(!buf) {
	    DEBUG('a',"Can't allocate kernel buffer in Print\n");
	  }
	  
	  if(copyin(virtualAddress_p, 128, buf_p)==-1) {
	    DEBUG('a',"Bad pointer passed to Print\n");
	    delete[] buf;
	  }
	  buf_p[128] = '\0';
	  printf(buf_p,p1,p2,p3);
	  break;
	case SC_Register:

	  Register_Syscall();
	  break;

	case SC_GetMV:

	  GetMV_Syscall(machine->ReadRegister(4));
	  break;

	case SC_SetMV:

	  SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	  break;

	case SC_CreateMV:

	  CreateMV_Syscall(machine->ReadRegister(4));
	  break;
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else if( which == PageFaultException) {
      DEBUG('f',"Page Fault Exception\n");
      int vaddress;
      vaddress = machine->ReadRegister(39);
      int vpnumber = vaddress / PageSize; 
      if(vpnumber > PageSize*NumPhysPages) {
	printf("VPN is OUT OF BOUNDS\n");
      }
      unsigned int index;
  
      // Check to see if the page is in the IPT
      int i;
     
      
      for(i = 0; i < NumPhysPages; i++) {
	// If the processId in the IPT is the same as the current thread's address space id
	// we know they are from the same process 
	// printf("process id is: %d, currentThread space id is: %d\n",machine->ipt[i].processId,currentThread->space->id);
	
	if(ipt[i].processId == currentThread->space->id) {
	  
      	  // If the virtual page is the same as the current thread's address space's virtual page 
	  // then we know we found the right page
	  if(ipt[i].virtualPage == vpnumber) {
	    // THIS IS AN IPT HIT
	    // We have the virtual page 
	    // UPDATE THE TLB CODE
	    DEBUG('c',"ipt virtual page and vpn is the same\n");

	    for(int z = 0; z < NumPhysPages; z++) {
	      if(machine->tlb[tlbCounter].virtualPage == ipt[z].virtualPage) {
		if(machine->tlb[tlbCounter].dirty == TRUE) {
		  ipt[z].dirty = TRUE;
		}
	      }
	    }
	    
	    machine->tlb[tlbCounter].physicalPage = ipt[i].physicalPage;
	    machine->tlb[tlbCounter].virtualPage  = ipt[i].virtualPage;
	    machine->tlb[tlbCounter].valid        = ipt[i].valid;
	    machine->tlb[tlbCounter].use          = ipt[i].use;
	    machine->tlb[tlbCounter].dirty        = ipt[i].dirty;
	    break;
	  } 
	  // If we get here, the page we are looking for is not in memory, so we have to load 
	  // from the executable or perhaps the swap file 
	}
	if(i == NumPhysPages-1) {
	  // This is an IPT Miss 
	  index = bitmap->Find();

	  // Main memory is full, as is the IPT
	  if(index == -1) {
	    DEBUG('c',"Main memory is full\n");
	    int evictPage;
	    // The page to be evicted is the first one in FIFO data structure
	    evictPage = fifo[0];
	    DEBUG('c',"evicting page %d\n",evictPage);
	    // Check if this page has an entry in the TLB
	    for(i = 0; i < TLBSize; i++) {
	      if(evictPage == machine->tlb[i].physicalPage) {
		// Rewrite it
		IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable Interrupts
		machine->tlb[i].valid = FALSE;
		oldLevel = interrupt->SetLevel(oldLevel); // Disable Interrupts
	      }
	    }
	    
	    // Save the page being evicted, but only if it is dirty
	    if(ipt[evictPage].dirty == TRUE) {
	      DEBUG('g',"page %d is dirty\n", evictPage);
	      // Write it to the swap file
	      IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable Interrupts
	      swapFile->WriteAt(&(machine->mainMemory[evictPage*PageSize]),PageSize,swapCounter*PageSize);
	      //currentThread->space->pageTable[ipt[evictPage].virtualPage].location = 1;
	      //currentThread->space->pageTable[ipt[evictPage].virtualPage].swapLoc = swapCounter;
	      swapCounter++;	
	      interrupt->SetLevel(oldLevel); // Re-Enable Interrupts
	    }
	    
	    // Shift the entire fifo array
	    for(i = 1; i < NumPhysPages; i++) {
	      fifo[i-1] = fifo[i];
	    }
	    DEBUG('c',"fifocounter is: %d \n",fifoCounter);
	    fifo[fifoCounter] = evictPage;
	    
	    for(i = 0; i < NumPhysPages; i++) {
	      DEBUG('f',"fifo %d: %d\n",i,fifo[i]);
	    }

	    // If this page is inside the swap file	    
	    // The code underneath is wrong, but its okay since we're not using the TLB
	    if(currentThread->space->pageTable[vpnumber].physicalPage == 1){
	      DEBUG('c',"Page %d is in the swap file\n",evictPage);
	      //Load from swap file to main memory
	      IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable Interrupts

	      //swapFile->ReadAt(&(machine->mainMemory[evictPage*PageSize]),PageSize,currentThread->space->pageTable[vpnumber].swapLoc*PageSize);
	      interrupt->SetLevel(oldLevel); // Re-Enable Interrupts
	    } else {
	      // Load the new page from executable into memory
	      DEBUG('c',"load from executable into memory\n");
	      currentThread->space->memoryLoad(vpnumber, evictPage);
	    }	    

	    // Evict this page and put in the new page
	    ipt[evictPage].physicalPage = evictPage;
	    ipt[evictPage].virtualPage  = vpnumber;
	    ipt[evictPage].valid        = TRUE;
	    ipt[evictPage].use          = FALSE;
	    ipt[evictPage].dirty        = FALSE;
	    ipt[evictPage].readOnly     = FALSE;
	    ipt[evictPage].processId    = currentThread->space->id;
	    
	    for(int z = 0; z < NumPhysPages; z++) {
	      if(machine->tlb[tlbCounter].virtualPage == ipt[z].virtualPage) {
		if(machine->tlb[tlbCounter].dirty == TRUE) {
		  ipt[z].dirty = TRUE;
		}
	      }
	    }

	    // UPDATE THE TLB CODE
	    machine->tlb[tlbCounter].physicalPage = evictPage;
	    machine->tlb[tlbCounter].virtualPage  = vpnumber;
	    machine->tlb[tlbCounter].valid        = TRUE;
	    machine->tlb[tlbCounter].use          = currentThread->space->pageTable[vpnumber].use;
	    machine->tlb[tlbCounter].dirty        = currentThread->space->pageTable[vpnumber].dirty;	   

	  } else {
	    // Main memory has space
	    // Update the IPT CODE
	    DEBUG('c',"the page is not inside the ipt\n");
	    ipt[index].physicalPage = index;
	    ipt[index].virtualPage  = vpnumber;
	    ipt[index].valid        = TRUE;
	    ipt[index].use          = FALSE;
	    ipt[index].dirty        = FALSE;
	    ipt[index].readOnly     = FALSE;
	    ipt[index].processId    = currentThread->space->id;
	    
	    fifo[fifoCounter] = index;
	    if(fifoCounter < NumPhysPages-1) {
	      fifoCounter++;
	    }
	    if(fifoCounter >= NumPhysPages) {
	      DEBUG('c',"fifo counter is larger than numphyspages\n");
	    }
	    currentThread->space->pageTable[vpnumber].physicalPage = index;

	    for(int z = 0; z < NumPhysPages; z++) {
	      if(machine->tlb[tlbCounter].virtualPage == ipt[z].virtualPage) {
		if(machine->tlb[tlbCounter].dirty == TRUE) {
		  ipt[z].dirty = TRUE;
		}
	      }
	    }
	    
	    // UPDATE THE TLB CODE
	    machine->tlb[tlbCounter].physicalPage = currentThread->space->pageTable[vpnumber].physicalPage;
	    machine->tlb[tlbCounter].virtualPage  = currentThread->space->pageTable[vpnumber].virtualPage;
	    machine->tlb[tlbCounter].valid        = currentThread->space->pageTable[vpnumber].valid;
	    machine->tlb[tlbCounter].use          = currentThread->space->pageTable[vpnumber].use;
	    machine->tlb[tlbCounter].dirty        = currentThread->space->pageTable[vpnumber].dirty;	    
	    
	    // Load it into memory
	    currentThread->space->memoryLoad(vpnumber, index);
	  }
	}   
      }   
      
      if(tlbCounter == TLBSize-1) {
	tlbCounter = 0;
      } else {
	tlbCounter++;
      }
      return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}

RequestType getNetThreadRequestType(char* req) {
  
  if(strcmp(req, "L")==0) {
    return CREATE_LOCK;
  } else if(strcmp(req, "A")==0) {
    return ACQUIRE;
  } else if (strcmp(req, "R")==0) {
    return RELEASE;
  } else if (strcmp(req, "D")==0) {
    return DESTROY_LOCK;
  } else if (strcmp(req, "C")==0) {
    return CREATE_CONDITION;
  } else if (strcmp(req, "W")==0) {
    return WAIT;
  } else if (strcmp(req, "S")==0) {
    return SIGNAL;
  } else if (strcmp(req, "B")==0) {
    return BROADCAST;
  } else if (strcmp(req, "J")==0) {
    return REGISTER;
  } else if (strcmp(req, "CMV")==0) {
    return CREATE_MV;
  } else if (strcmp(req, "GMV")==0) {
    return GET;
  } else if (strcmp(req, "SMV")==0) {
    return SET;
  } else if (strcmp(req, "T")==0) {
    return TOKEN;
  } else if (strcmp(req, "X")==0) {
    return REGISTER_RESPONSE;
  } else {
    return UNKNOWN;
  }

}
