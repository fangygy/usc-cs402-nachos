void CheckInStaff() {
  
  int myNumber;
  
  Acquire(cisNumLock);
  myNumber = cisNum;
  cisNum++;
  Release(cisNumLock);

  if((current_test == 1)||(current_test==2)||(current_test == 3)) {
    /* currentThread->Finish(); */
    Exit(0);
  } 
  while(1) {
    int myAirline;
    if(myNumber > 9) {
      myAirline = 2;
    } else if (myNumber > 4) {
      myAirline = 1;
    } else {
      myAirline = 0;
    }
    
    /* Acquire the line lock
       cisLineLock[myAirline]->Acquire(); */

    /*
     * if there is no one in line 
     * check in staff goes on break
     *
     * there are people in line 
     * signal someone to come into the line
     * wait for them to hand their ticket
     * generate boarding pass
     * signal passenger with boarding pass
     * 
     */

    /* cisLineLock[myAirline]->Acquire(); */
    Acquire(cisLineLock[myAirline]);
    if(cisLineLengths[myNumber]==0 && execLineLengths[myAirline]==0) {
      /* go on break */
      /* onBreakCIS_C[myNumber]->Wait(cisLineLock[myAirline]); */
      Wait(onBreakCIS_C[myNumber], cisLineLock[myAirline]);
    }
    
    /* If executive line > 0
       Help the executives */
    
    /* execLineLock[myAirline]->Acquire(); */
    if(execLineLengths[myAirline] > 0) {
      int flight_number = 0;
      /* execLineLock[myAirline]->Acquire(); */
      Acquire(execLineLock[myAirline]);
      waitingForExec[myNumber] = 1;
      /* Tell an executive that I am ready */
      /* execLineCV[myAirline]->Signal(execLineLock[myAirline]); */
      Signal(execLineCV[myAirline], execLineLock[myAirline]);
      /* Now waiting for the executive to signal 
         execLineCV[myAirline]->Wait(execLineLock[myAirline]); */
    
      /* execCISLock[myNumber]->Acquire(); */
      Acquire(execCISLock[myNumber]);
      /* execLineLock[myAirline]->Release(); */
      Release(execLineLock[myAirline]);

      /*waitingForExec_CIS_C[myNumber]->Wait(execCISLock[myNumber]);
        waitingForExec_CIS_C[myNumber]->Signal(execCISLock[myNumber]);*/
      Wait(waitingForExec_CIS_C[myNumber], execCISLock[myNumber]);
      Signal(waitingForExec_CIS_C[myNumber], execCISLock[myNumber]);
 
      cisFlightCount[myAirline]++;
      cisPassengerCount++;


      flight_number = pass_ticket_buffer[cisPassenger[myNumber]].flight_number;   
      
      Print("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n",myNumber,myAirline,cisLineLengths[myNumber]);
      Print("Airline check-in staff %d of airline informs executive passenger %d to board at gate %d\n",myNumber,cisPassenger[myNumber], flight_number);
      

      /* Add these bags to the total count fort a given airline, specified by Flight Number */
      cis_baggage_buffer[flight_number] += baggage_buffer[cisPassenger[myNumber]].weight;
      
      /* Now add these bags to the conveyor belt */
      conveyorBelt[cisPassenger[myNumber]].airline_code = flight_number;
      conveyorBelt[cisPassenger[myNumber]].number_of_bags = 2;
      conveyorBelt[cisPassenger[myNumber]].weight = baggage_buffer[cisPassenger[myNumber]].weight;
      /*
      printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n", myNumber,myAirline);
      */

      /* CIS not waiting for executive passenger anymore */
      waitingForExec[myNumber]=0;
      /* execCISLock[myNumber]->Release(); */
      Release(execCISLock[myNumber]);
    }
    /* execLineLock[myAirline]->Release();
       execCISLock[myNumber]->Release(); */

    /* cisLineLock[myAirline]->Acquire(); */

    if(cisLineLengths[myNumber] > 0) {
      int flight_number = 0;
      /* printf("line %d has more than one passenger\n", myNumber);
         cisLineLock[myAirline]->Acquire(); */
      /* waitingForCIS_C[myNumber]->Signal(cisLineLock[myAirline]); */
      Signal(waitingForCIS_C[myNumber], cisLineLock[myAirline]);
      /* printf("%s telling Passenger %d to come to counter\n", currentThread->getName(), cis_current_passenger_serving[myNumber]); */
      /* cisLock[myNumber]->Acquire(); */

      Acquire(cisLock[myNumber]);
      
      /* cisLineLock[myAirline]->Release(); */
      Release(cisLineLock[myAirline]);
      

      /*waitingForTicket_CIS_C[myNumber]->Wait(cisLock[myNumber]);
        waitingForTicket_CIS_C[myNumber]->Signal(cisLock[myNumber]);*/
      Wait(waitingForTicket_CIS_C[myNumber], cisLock[myNumber]);
      Signal(waitingForTicket_CIS_C[myNumber], cisLock[myNumber]);

      flight_number = pass_ticket_buffer[cisPassenger[myNumber]].flight_number;   

      
      Print("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",myNumber,myAirline,execLineLengths[myNumber]);
      
      /* Give the Passenger a seat number */
      boarding_pass_buffer[cisPassenger[myNumber]].seat_number = seatNumber[flight_number]++;
      
      Print("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate ",myNumber,myAirline,cisPassenger[myNumber]);
      Print("%d\n",flight_number,0,0);
      
      /* Add these bags to the total count fort a given airline, specified by Flight Number */
      cis_baggage_buffer[flight_number] += baggage_buffer[cisPassenger[myNumber]].weight;
      
      /* Now add these bags to the conveyor belt */
      conveyorBelt[cisPassenger[myNumber]].airline_code   = flight_number;
      conveyorBelt[cisPassenger[myNumber]].number_of_bags = baggage_buffer[cisPassenger[myNumber]].numberOfBags;
      conveyorBelt[cisPassenger[myNumber]].weight         = baggage_buffer[cisPassenger[myNumber]].weight;

      
      Print("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n",myNumber,myAirline,0);
      cisFlightCount[myAirline]++;
      cisPassengerCount++;
    }
    /* cisLock[myNumber]->Release();*/
    Release(cisLock[myNumber]);
  }
  Exit(0);
}

void main(){
