void AirportManager() {
  int myNumber;

  myNumber = 1;
  if(current_test > 0) {
    /*currentThread->Finish();*/
    Exit(0);
  }
  
  while(1) {
    
    /* if all passengers are accounted for
       issue broadcast */
    int airlineCounter = 0;
    int tempCount = 0;

    /*conveyorBelt_Lock.Acquire();*/
    Acquire(conveyorBelt_Lock);
    if(onBreak_CH==1) {
      int passengerCounter = 0;
      for(passengerCounter = 0; passengerCounter < numberOfPassengers; passengerCounter++) {
        if(conveyorBelt[passengerCounter].number_of_bags == 0) {
          /* check next bag */
        } else {
          /* printf("Airport Manager calls back all the cargo handlers from break\n");*/
          /*onBreakCH.Broadcast(&conveyorBelt_Lock);*/
          Broadcast(onBreakCH, conveyorBelt_Lock);
          break;
        }
      }
    }
    /*conveyorBelt_Lock.Release();*/
    Release(conveyorBelt_Lock);
    /* int airlineCounter = 0; */
    for(airlineCounter = 0; airlineCounter < numberOfAirlines; airlineCounter++) {
      int secAirlineCounter = 0;

      /*airlineLock[airlineCounter]->Acquire();*/
      Acquire(airlineLock[airlineCounter]);
      if(alreadyCalled[0]==1&&alreadyCalled[1]==1&&alreadyCalled[2]==1) {
        /* TODO
           print out statistics */
        /*
        printf("Passenger count reported by airport liason = %d\n",alPassengerCount);
        printf("Passenger count reported by airline check-in staff = %d\n",cisPassengerCount);
        printf("Passenger count reported by security inspector = %d\n",sicount);
        */
        /*for(int g = 0; g < numberOfPassengers; g++) {
          printf("[%d, %d]",g,conveyorBelt[g].number_of_bags);
        }
        printf("\n"); */
	


        for(secAirlineCounter = 0; secAirlineCounter < numberOfAirlines; secAirlineCounter++) {
          
          /*printf("----------------Statistics--------------\n");*/
          Print("From setup: Baggage count of airline %d = %d\n",secAirlineCounter,numBagsDuringSetup[secAirlineCounter],0);
          Print("From airport liason: Baggage count of airline %d = %d\n",secAirlineCounter, al_baggage_buffer[secAirlineCounter],0);
          Print("From cargo handlers: Baggage count of airline %d = %d\n",secAirlineCounter, cargoHandlerBaggageCount[secAirlineCounter],0);
          Print("From setup: Baggage weight of airline %d = %d\n",secAirlineCounter,bagWeightsDuringSetup[secAirlineCounter],0);
          Print("From airline check-in staff: Baggage weight of airline %d = %d\n",secAirlineCounter, cis_baggage_buffer[secAirlineCounter],0);
          Print("From cargo handlers: Baggage weight of airline %d = %d\n",secAirlineCounter,chBagWeights[secAirlineCounter],0);
          
        }
        /*goToSleep.Wait(airlineLock[airlineCounter]);*/
        Wait(goToSleep, airlineLock[airlineCounter]);
      }
      /*printf("flight %d count %d , cisflightcount %d\n",i,flightCount[i],cisFlightCount[i]); */

      if(!alreadyCalled[airlineCounter]==1&&(flightCount[airlineCounter] == cisFlightCount[airlineCounter])&&(flightCount[airlineCounter]!=0)&&(cisFlightCount[airlineCounter]!=0)&&(cargoHandlerBaggageCount[airlineCounter]==al_baggage_buffer[airlineCounter])) {
        Print("Airport Manager gives a boarding call to airline %d\n",airlineCounter,0,0);
        /* waitingForCallAM_C[airlineCounter]->Broadcast(airlineLock[airlineCounter]); */
        Broadcast(waitingForCallAM_C[airlineCounter], airlineLock[airlineCounter]);

        alreadyCalled[airlineCounter] = 1;
      } else {
        
      }

      /*airlineLock[airlineCounter]->Release();*/
      Release(airlineLock[airlineCounter]);
    }
    /* int tempCount = 0; */
    for(tempCount = 0; tempCount < 10; tempCount++) {
      /*currentThread->Yield();*/
      Yield();
    }
  }
}

void main(){
