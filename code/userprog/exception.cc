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

using namespace std;

#define MAX_CHARS 100;

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

int CreateLock_Syscall(int name, int size) {
 
  KernelLockTableLock->Acquire();

  // Make sure the table is not full 
  if(nextLockIndex >= MAX_LOCKS) {
    //The table is full of locks 
    KernelLockTableLock->Release();
    DEBUG('a',"LOCK TABLE FULL");
    return -1;
  }

  //in the clear to create the lock
  char *lockName = new char[size+1];
  lockName[size] = '\0';
  //copy into
  // copyin(lockName, name, size);
 
  //initialize important vars
  osLocks[nextLockIndex].lock = new Lock(lockName);
  osLocks[nextLockIndex].as = currentThread->space;
  //initialize lock to not in use
  osLocks[nextLockIndex].usageCounter = 0;
  //no one wants to destroy the lock right now
  osLocks[nextLockIndex].toBeDestroyed = FALSE;
  //report number of locks to user first...
  //increment number of locks
  nextLockIndex++;
  KernelLockTableLock->Release();
  return nextLockIndex; // this may prove to have some problems if
                        // we are context switched out before nextLockIndex is
                        // returned


 
}

void DestroyLock_Syscall(int index) {
  // Delete from kernel structure array the lock object at position index
}

void Acquire_Syscall(int index) {
  int value = index; // this is the value read by machine->readregister(4)
  KernelLockTableLock->Acquire();
  DEBUG('a',"ACQUIRING LOCK\n\n\n\n\n");
  // perform series of checks on the lock
  // to make sure user program is not doing
  // anything crazy

  //make sure the lock exists
  if(value < 0 || value >= nextLockIndex) {
    // This is a bad value
    DEBUG('a',"BAD VALUE\n");
    return;
  }

  KernelLock curLock = osLocks[value];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('a',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  // The lock has not been destroyed
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space
    DEBUG('a',"LOCK BELONGS TO DIFFERENT PROCESS");
    return;
  }
  //ensure that lock isn't destroyed while in use
  curLock.usageCounter++; 
  
  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
  // FINALLY...Acquire the lock
  curLock.lock->Acquire();
}

void Release_Syscall(int index) {
  //NOTE: I just made this the same checks as Acquire for now....
  int value = index;
  KernelLockTableLock->Acquire();
  //make sure the lock exists
  if(value < 0 || value >= nextLockIndex) {
    // This is a bad value
    DEBUG('a',"BAD VALUE\n");
    return;
  }
  KernelLock curLock = osLocks[index];
  if(curLock.lock == NULL) {
    // The lock has been destroyed 
    DEBUG('a',"LOCK HAS BEEN DESTROYED\n");
    return;
  }
  
  if(curLock.as != currentThread->space) {
    // this lock belongs to a different process
    // since the address space of the lock does not match the 
    // current thread's address space    
    DEBUG('a',"LOCK BELONGS TO DIFFERENT PROCESS");
    return;
  }
  //ensure that lock isn't destroyed while in use
  curLock.usageCounter++; 
  
  //has to go above acquire to avoid deadlock
  KernelLockTableLock->Release();
  // FINALLY...Release the lock
  curLock.lock->Release();
}

int CreateCondition_Syscall() {
  
  /* Fix this
    if(name < 0 || (name+size) >= addressSpaceSize) {

    }
  
    char *condName = new char[size+1];
    condName[size] = '\0';
    copyin(condName, name, size);
    
  */
  KernelCondTableLock->Acquire();
  if(nextCondIndex >= MAX_CONDS) {
    DEBUG('a', "OUT OF BOUNDS ERROR\n"); 
  }
  osConds[nextCondIndex].condition = new Condition("some name");
  osConds[nextCondIndex].as   = currentThread->space;
  osConds[nextCondIndex].usageCounter = 0;
  osConds[nextCondIndex].toBeDestroyed = FALSE;
  nextCondIndex++;
  KernelCondTableLock->Release();
  return nextCondIndex; 
}

void DestroyCondition_Syscall(int index) {
  // Delete from kernel structure array the condition object at position index 

}

void Wait_Syscall(int index, int lock_id) {
  KernelCondTableLock->Acquire();
  /*
    Check bounds, check lock is valid 
    
  */
  
  KernelCond curCond = osConds[index];
	if(curCond.as != currentThread->space) {
     // return and print a message
	}
  
  KernelLock curLock = osLocks[lock_id];

  curCond.condition->Wait(curLock.lock); // may get an error here due to pointer usage
  KernelCondTableLock->Release();
}

void Signal_Syscall(int index, int lock_id) {
  KernelCondTableLock->Acquire();
  /*
    Check bounds, check lock is valid 
    
  */
  
  KernelCond curCond = osConds[index];
  // check address space
  if(curCond.as != currentThread->space) {
     // return and print a message
  }
  
  KernelLock curLock = osLocks[lock_id];

  curCond.condition->Signal(curLock.lock); // may get an error here due to pointer usage
  KernelCondTableLock->Release();


}

void Broadcast_Syscall(int index, int lock_id) {

  KernelCondTableLock->Acquire();
  /*
    Check bounds, check lock is valid 
    
  */
  
  KernelCond curCond = osConds[index];
	if(curCond.as != currentThread->space) {
		
	}
  
  KernelLock curLock = osLocks[lock_id];

  curCond.condition->Broadcast(curLock.lock); // may get an error here due to pointer usage
  KernelCondTableLock->Release();
}

void Yield_Syscall() {
  currentThread->Yield();
}

void Exit_Syscall(int status) {
  //TODO - finish this
  //interrupt->Halt();
  currentThread->Finish();
}

void execThread() {
  DEBUG('f',"running the thread 1\n");
  currentThread->space->InitRegisters();
  DEBUG('f',"running the thread 2 \n");
  currentThread->space->RestoreState();
  machine->Run();
  DEBUG('f',"running the thread\n");
}

void kernelFunc(int vaddr) {
  int i, spaceId;
  for(i = 0; i < 64; i++) {
    if(currentThread->space == processTable[i].as) {
      spaceId = i;
      break;
    } else {

    }
  }
  // write to register PCReg the virtual address
  machine->WriteRegister(PCReg, vaddr);
  
  // write virtual address + 4 in NextPCReg
  machine->WriteRegister(NextPCReg, (vaddr+4));
  // call RestoreState function
  currentThread->space->RestoreState();
  // write to stack register, the starting position of the stack
  machine->WriteRegister(StackReg,processTable[spaceId].stackLocation);
		
  currentThread->setStack((int *)processTable[spaceId].stackLocation);
  // allocate address space to the thread
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
	        rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
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
	        DEBUG('a', "Exit syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Fork:
	        DEBUG('a', "Fork syscall.\n");
		// get the virtual address of function being forked
		int virtualAddress;
		virtualAddress = machine->ReadRegister(4);
		int i, spaceId_f;
		// get the space id for this new thread
		for(i=0; i<64; i++) {
		  if(processTable[i].as == currentThread->space) {
		    spaceId_f = i;
		    break;
		  } else { 
		    // Trying to fork a thread without an existing address space
		    DEBUG('a', "Trying to fork a thread without an existing address space\n");
		  }
		}
		Thread *kernelThread = new Thread("kernelThread");
		// this is the same as the currentThread->space b/c this thread
		// is a child of the currentThread
		kernelThread->space = currentThread->space;
		// Create a new page table with 8 pages more of stack
		kernelThread->space->NewPageTable();

		// Update page table
		processTable[spaceId_f].stackLocation = (kernelThread->space->NumPages()*PageSize)-16;
		processTable[spaceId_f].numChildProcess++;
		kernelThread->Fork(kernelFunc,virtualAddress);
		
		break;
	    case SC_Exec:
	        DEBUG('a',"Exec syscall. \n");
		
		int virtualAddress_e, physicalAddress_e; 
		char* filename;
		// Get the virtual address for the name of the process
		virtualAddress_e = machine->ReadRegister(4);
		char *buf = new char[16+1];	// Kernel buffer to put the name in
		OpenFile *f;			// The new open file
		int id;				// The openfile id
		
		if (!buf) {
		  DEBUG('a',"Can't allocate kernel buffer in Open\n");
		 
		}
		
		if( copyin(virtualAddress_e,16,buf) == -1 ) {
		  DEBUG('a',"Bad pointer passed to Open\n");
		  delete[] buf;
		}
		
		buf[16]='\0';

		// Print out filename
		printf("%s\n",buf);

		f = fileSystem->Open(buf);
		if(f == NULL) {
		  printf("%s","unable to open file\n");
		  
		} else {

		  AddrSpace *space;
		  DEBUG('a',"Got the file open\n");
		  
		  // create a new address space for this executable file
		  space = new AddrSpace(f);
		  Thread *executionThread = new Thread("executionThread");
		  // Allocate the address space to this thread
		  executionThread->space = space;
		  
		  DEBUG('a',"allocated the address space\n");
		  
		  // Update process table
		  int g, spaceId;
		  for(g = 0; g < 64; g++) {
		    if(!processTable[g].inUse) {
		      spaceId = g;
		      // Set the appropriate address space
		      processTable[spaceId].as = space;
		      processTable[spaceId].stackLocation = (space->NumPages()*PageSize) - 16;
		      break;
		    }
		  }
		  
		  DEBUG('a',"Updated the process table with new process\n");
		  
		  // Write the space id to register 2
		  rv = spaceId;
		  // Fork the thread
		  executionThread->Fork((VoidFunctionPtr)execThread,0);
		}
		break;
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
