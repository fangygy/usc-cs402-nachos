void SecurityInspector() {
  int myNumber;
  Acquire(securityInsNumLock);
  myNumber = securityInsNum;
  securityInsNum++;
  Release(securityInsNumLock);

  while(1) {
    

    /* siLineLock.Acquire(); */
    Acquire(siLineLock);
    
    if((siLineLengths[myNumber]==0) && (siLineReturns[myNumber]==0)) { /* && returning line */
      /*waitingSI_C[myNumber]->Wait(&siLineLock);*/
      Wait(waitingSI_C[myNumber], siLineLock);
      Print("Security Inspector %d has someone in their line\n",myNumber,0,0);
    }
    
    /* If people returning line > 0
       Help them first */
    
    /* execLineLock[myAirline]->Acquire(); */
    while(siLineReturns[myNumber] > 0) {
      Print("some asshole is in the returns line\n",0,0,0);
      /* siReturnLock[myNumber]->Acquire(); */
      Acquire(siReturnLock[myNumber]);

      /* Tell an executive that I am ready */
      /*returnLineCV[myNumber]->Signal(siReturnLock[myNumber]);*/
      Signal(returnLineCV[myNumber], siReturnLock[myNumber]);
      /* Now waiting for the executive to signal 
         execLineCV[myAirline]->Wait(execLineLock[myAirline]); */
    
      /*siRLock[myNumber]->Acquire(); */
      Acquire(siRLock[myNumber]);
      /* siReturnLock[myNumber]->Release(); */
      Release(siReturnLock[myNumber]);

      /*waitingForReturn_SI_C[myNumber]->Wait(siRLock[myNumber]);
        waitingForReturn_SI_C[myNumber]->Signal(siRLock[myNumber]);*/
      Wait(waitingForReturn_SI_C[myNumber], siRLock[myNumber]);
      Signal(waitingForReturn_SI_C[myNumber], siRLock[myNumber]);
      Print("Security inspector %d permits returning passenger %d to board\n", myNumber, siPassenger[myNumber],0);
       
      /* increment si count of passengers?*/
      sicount++;
      /* siLineReturns[myNumber]--; */
      /* siRLock[myNumber]->Release(); */
      Release(siRLock[myNumber]);
    }

    if(siLineLengths[myNumber] > 0) {
      int passedSI = 1; /* changed from bool */
      int randomNum = 76;
      Print("Security Inspector %d telling passenger to come to counter \n",myNumber,0,0); 
      /*waitingForSI_C[myNumber]->Signal(&siLineLock);*/
      Signal(waitingForSI_C[myNumber], siLineLock);
     /* siLock[myNumber]->Acquire(); */
      Acquire(siLock[myNumber]);

      /* siLineLock.Release(); */
      Release(siLineLock);
      
      /*waitingForTicket_SI_C[myNumber]->Wait(siLock[myNumber]);*/
      Print("Security Inspector %d before waitingforticket\n",myNumber,0,0);
      
      Wait(waitingForTicket_SI_C[myNumber], siLock[myNumber]);
      /*waitingForTicket_SI_C[myNumber]->Signal(siLock[myNumber]);*/
      Signal(waitingForTicket_SI_C[myNumber], siLock[myNumber]);
      
      Print("Security inspector %d allows passenger %d to board\n", myNumber,siPassenger[myNumber],0);    
      
      sicount++;
      /* printf("si has moved %d passengers\n",sicount);
	 Keep track of how many passengers are cleared for each airline */
      
    }
    /* siLock[myNumber]->Release(); */
    Release(siLock[myNumber]);
  
  }
}

void main(){
