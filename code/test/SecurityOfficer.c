void SecurityOfficer() {
  int myNumber;

  Acquire(securityOffNumLock);
  myNumber = securityOffNum;
  securityOffNum++;
  Release(securityOffNumLock);

  while(1) {

    /* Acquire the line lock

    /* soLineLock.Acquire(); */
    Acquire(soLineLock);
    if(soLineLengths[myNumber]==0) {
      /* go on break */
      /* waitingSO_C[myNumber]->Wait(&soLineLock); */
      Wait(waitingSO_C[myNumber], soLineLock);
    }
    
    if(soLineLengths[myNumber] > 0) {
      int passenger_line = 0;
      /* waitingForSO_C[myNumber]->Signal(&soLineLock); */
      Signal(waitingForSO_C[myNumber], soLineLock);
      /* soLock[myNumber]->Acquire(); */
      Acquire(soLock[myNumber]);
      /* soLineLock.Release(); */
      Release(soLineLock);
      
      /* Determine if the passenger passes or fails */
      
      so_passOrFail[myNumber] = 1;

      if((soPassenger[myNumber]==3)||(soPassenger[myNumber]==13)||(soPassenger[myNumber]==17)) {
        /*printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n", myNumber,soPassenger[myNumber]);*/

      } else {
        /*printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n", myNumber,soPassenger[myNumber]);*/
      }

      so_passOrFail[soPassenger[myNumber]] = 1;

      /* Screening Officer looks for an available line
       If he cannot find one, he sends the passenger to the Security Officer
       with the shortest line */
      passenger_line = findShortestLine(siLineLengths,7);
      passengerGoToSI[ soPassenger[myNumber] ] = passenger_line;

      /* waitingForTicket_SO_C[myNumber]->Wait(soLock[myNumber]);
         waitingForTicket_SO_C[myNumber]->Signal(soLock[myNumber]); */
      Wait(waitingForTicket_SO_C[myNumber], soLock[myNumber]);
      Signal(waitingForTicket_SO_C[myNumber], soLock[myNumber]);
      Print("Screening officer %d directs passenger %d to security inspector %d\n", myNumber, soPassenger[myNumber], passengerGoToSI[ soPassenger[myNumber] ]);
      numbersopassed++;
      Print("number so passed: %d\n",numbersopassed,0,0);
    }
    /* soLock[myNumber]->Release(); */
    Release(soLock[myNumber]);
  }
}

void main(){
