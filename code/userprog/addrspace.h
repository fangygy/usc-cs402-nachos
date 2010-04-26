// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"
#include "bitmap.h"
#include "NewTranslationEntry.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;                    // Table of openfiles

    void NewPageTable();
    unsigned int NumPages();
    int id;
    void DeAllocate(int stackLocation);
    TranslationEntry *pageTable;	// Assume linear page table translation
    OpenFile* asExecutable;

    void memoryLoad(int vpnumber, int index);
    void setMailbox(int mbox) { mailbox = mbox; }
    int getMailbox() { return mailbox; }
 private:
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    int numCodePages, numInitPages;
    int mailbox;
};

class ChildProcess {
 public:
  ChildProcess();
  ~ChildProcess();
 private:
  int id; // child id 
  AddrSpace *childAS; // address space pointer of child
  //int numOfChildThreads; // counter for number of child threads
  // some synch primitive -- won't need this since not implementing join
  bool exiting; // indication variable for exiting -- won't need this since not implementing join
  // Thread* parentThread; // parent thread pointer
};

class ProcessTable {
 public:
  ProcessTable();
  ~ProcessTable();
  int numChildProcess;
  AddrSpace *as; // address pointer of parents
  int spaceId; 
  int stackLocation;
  // ChildProcess *childProcess;
  bool inUse;
  
 private:

};
#endif // ADDRSPACE_H
