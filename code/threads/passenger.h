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
  // printf("Acquiring Lock...");
  cisLineLock.Acquire();

  // Set the Passenger's line number
  myLineNumber = findShortestLine(cisLineLengths, 5);

 
  cisLineLengths[myLineNumber]++;
  onBreakCIS_C[myLineNumber]->Signal(&cisLineLock);
  printf("%s chose Airline Check In %d with length %d\n", currentThread->getName(), myLineNumber, cisLineLengths[myLineNumber]);
  waitingForCIS_C[myLineNumber]->Wait(&cisLineLock);
  cisLineLengths[myLineNumber]--;

  printf("%s going to see Airline Check In Staff %d\n",currentThread->getName(), myLineNumber); 
  cisLineLock.Release();
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
}
