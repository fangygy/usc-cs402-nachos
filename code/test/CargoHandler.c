void CargoHandler() {
  int myNumber;
  Acquire(cargoHandlerNumLock);
  myNumber = cargoHandlerNum;
  cargoHandlerNum++;
  Release(cargoHandlerNumLock);

  while(1) {
    int passCounter = 0;
    /*conveyorBelt_Lock.Acquire();*/
    Acquire(conveyorBelt_Lock);
    /*
      if(on break is true)
      onbreakch->wait(conveyorbelt_lock)
      if(conveyor belt = empty)
      set onbreak to true
      
      take the baggage off of the conveyor belt
      airline_CH_Lock->Acquire()
      conveyorBelt_Lock->Release()
      cargoHandlerBaggageCount[baggage.flight_number]+=baggage.numberofbags

      airline_CH_Lock->Release()

     */
    
    if(onBreak_CH==1) {
      /*printf("Cargo Handler %d is going on a break\n",myNumber);*/
      if(current_test == 6) {
        /*conveyorBelt_Lock.Release();*/
        Release(conveyorBelt_Lock);
        /*currentThread->Finish();*/
        Exit(0);
      } else {
        /*onBreakCH.Wait(&conveyorBelt_Lock);*/
        Wait(onBreakCH, conveyorBelt_Lock);
      }
    }
    
    for(passCounter = 0; passCounter < numberOfPassengers; passCounter++) {
      if(conveyorBelt[passCounter].number_of_bags > 0) {
        /* Cargo Handler Found a bag */
        /*
          printf("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n",myNumber,conveyorBelt[passCounter].airline_code,conveyorBelt[passCounter].weight);*/
        chBagWeights[conveyorBelt[passCounter].airline_code] += conveyorBelt[passCounter].weight;
        cargoHandlerBaggageCount[conveyorBelt[passCounter].airline_code]+=conveyorBelt[passCounter].number_of_bags;
        conveyorBelt[passCounter].number_of_bags = 0;
        conveyorBelt[passCounter].airline_code = -1;
        /*conveyorBelt_Lock.Release();*/
        Release(conveyorBelt_Lock);
        break;
      }
      /* Goes through entire array and can not find a single bag */
      if(passCounter == (numberOfPassengers-1)) {
        onBreak_CH = 1;
        /*onBreakCH.Wait(&conveyorBelt_Lock);*/
        Wait(onBreakCH, conveyorBelt_Lock);
        /* conveyorBelt_Lock.Release(); */
      }      
    }
    /* onBreak_CH = true; */
    /*conveyorBelt_Lock.Release();*/
    Release(conveyorBelt_Lock);

  }
}

void main(){
