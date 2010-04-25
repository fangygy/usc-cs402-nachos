/* halt.c
 *      Simple program to test whether running a user program works.
 *      
 *      Just do a "syscall" that shuts down the OS.
 *
 *      NOTE: for some reason, user programs with global data structures 
 *      sometimes haven't worked in the Nachos environment.  So be careful
 *      out there!  One option is to allocate data structures as 
 *      automatics within a procedure, but if you do this, you have to
 *      be careful to allocate a big enough stack to hold the automatics!
 */

#include "functions.h"
#include "syscall.h"
int a[3];
int b, c;

#ifdef CHANGED
#include "synch.h"
#endif

/*using namespace std;*/
/* ---------------------------------------------------------------------*/
/*
 * PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO 
 * RUN AIRPORT SIMULATIONS
 *
 *
 *
*/
/* ---------------------------------------------------------------------*/

/* Global variables */

#define numberOfPassengers 20
#define numberOfAL 7
#define numberOfCIS 15
#define numberOfSO 7
#define numberOfAirlines 3
#define numberOfCH 6

#define probabilityPassingSO 90
#define probabilityPassingSI 90

int current_test = 0;

int sicount = 0;
int socount = 1;
int pass_si_count = 1;
int numBagsDuringSetup[numberOfAirlines];
int bagWeightsDuringSetup[numberOfAirlines];
int chBagWeights[numberOfAirlines];
int alPassengerCount = 0;
int cisPassengerCount = 0;

struct ticket {
  int passenger_number;
  int flight_number;
  int checkin_counter;
  int executive; /* changed from bool */
} pass_ticket_buffer[numberOfPassengers];

struct boarding_pass {
  int passenger_number;
  int flight_number;
  int seat_number;
} boarding_pass_buffer[numberOfPassengers];

struct baggage {
  int weight;
  int airline_code;
  int passenger_number;
  int numberOfBags;
  int weights[3];
} baggage_buffer[numberOfPassengers];

int passengerNumLock;
int airportLNumLock;
int cargoHandlerNumLock;
int securityOffNumLock;
int securityInsNumLock;
int cisNumLock;
int passengerNum;
int airportLNum;
int cargoHandlerNum;
int securityOffNum;
int securityInsNum;
int cisNum;
/* int passenger_baggage_buffer[numberOfPassengers];
// int cis_baggage_buffer[numberOfPassengers]; */

int al_baggage_buffer[numberOfAirlines];
int cis_baggage_buffer[numberOfAirlines];
int al_current_passenger_serving[numberOfAL]; /* must be equal to the number of airport liaisons */
int cis_current_passenger_serving[numberOfCIS];

/*Condition *waitingForCallAM_C[numberOfAirlines];*/
int waitingForCallAM_C[numberOfAirlines];
/*Lock *airlineLock[numberOfAirlines]; */
int airlineLock[numberOfAirlines];
int flightCount[numberOfAirlines];
int cisFlightCount[numberOfAirlines];
int alreadyCalled[numberOfAirlines]; /* changed from bool */
/* Condition goToSleep("goToSleep"); */
int goToSleep;

int totalweight = 0; /* for debugging */

/*Condition onBreakCH("ch_cv");*/
int onBreakCH;
/* Lock conveyorBelt_Lock("cb_lock"); */
int conveyorBelt_Lock;
/*Lock *airline_CH_Lock[numberOfAirlines];*/
/*int *airline_CH_Lock[numberOfAirlines]; -- not used? */
int onBreak_CH = 1; /* changed from bool */

struct conveyorBelt {
  int number_of_bags;
  int weight;
  int airline_code;
} conveyorBelt[numberOfPassengers];

int cargoHandlerBaggageCount[numberOfAirlines];

int passengersFailedSI[numberOfPassengers]; /* changed from bool */

int passengerGoToSI[numberOfPassengers];

int g;
g = 0;

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

/*Condition *waitingSI_C[numberOfSO];*/
int waitingSI_C[numberOfSO];
/*Condition *waitingForSI_C[numberOfSO];*/
int waitingForSI_C[numberOfSO];
/*Condition *waitingForTicket_SI_C[numberOfSO];*/
int waitingForTicket_SI_C[numberOfSO];
/*Condition *returnLineCV[numberOfSO];*/
int returnLineCV[numberOfSO];
/*Condition *waitingForReturn_SI_C[numberOfSO];*/
int waitingForReturn_SI_C[numberOfSO];

/* Lock siLineLock("si_LL"); */
int siLineLock;
/* Lock *siLock[numberOfSO]; */
int siLock[numberOfSO];
/* Lock siAirplaneCountLock("si_ALL"); */
/* int siAirplaneCountLock; - not used? */
/* Lock *siReturnLock[numberOfSO]; */
int siReturnLock[numberOfSO];
/* Lock *siRLock[numberOfSO]; */
int siRLock[numberOfSO];

int siLineReturns[numberOfSO];
int siLineLengths[numberOfSO];
int si_busy[numberOfSO]; /* changed from bool */
int so_passOrFail[numberOfPassengers]; /* changed from bool */
int siPassenger[numberOfSO];
int siAirlineCount[numberOfAirlines];

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

/* Condition *waitingSO_C[numberOfSO]; */
int waitingSO_C[numberOfSO];
/* Condition *waitingForSO_C[numberOfSO]; */
int waitingForSO_C[numberOfSO];
/* Condition *waitingForTicket_SO_C[numberOfSO]; */
int waitingForTicket_SO_C[numberOfSO];
/* Lock soLineLock("sl_LL"); */
int soLineLock;
/* Lock *soLock[numberOfSO]; */
int soLock[numberOfSO];
int soLineLengths[numberOfSO];
int so_busy[numberOfSO]; /* changed from bool */
int soPassenger[numberOfSO];
int numbersopassed = 0;

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

/* Objects for Airport Liaison */
/* Condition *waitingForAL_C[numberOfAL]; */
int waitingForAL_C[numberOfAL];
/* Condition *waitingForTicket_AL_C[numberOfAL]; */
int waitingForTicket_AL_C[numberOfAL];
/* Lock alLineLock("al_LL"); */
int alLineLock;
/* Lock *alLock[numberOfAL]; */
int alLock[numberOfAL];
int alLineLengths[numberOfAL];
int al_busy[numberOfAL]; /* changed from bool */
int alPassenger[numberOfAL];
/* int alPassengerCount = 0; */

void AirportLiaison() {
  int myNumber;
  
  Acquire(airportLNumLock);
  myNumber = airportLNum;
  airportLNum++;
  Release(airportLNumLock);

  while(1) {
    int flight_number = 0;
    /* Acquire the Line Lock
     No one can acquire the line lock (not Passengers)
     When the Airport Liaison has the LineLock, Passengers cannot search for the shortest
     line */
    /* alLineLock.Acquire(); */
    Acquire(alLineLock);

    /* If there are passengers in the line, then
     the Airport Liaison must tell the Passengerto step up to the counter
     He does this by Signaling the Condition Variable which puts the first Passenger
     on to the Ready Queue */
    if(alLineLengths[myNumber]>0) {
      /* The first passenger waiting for the LineLock gets put on to the Ready Queue */
      /* waitingForAL_C[myNumber]->Signal(&alLineLock); */
      Print("Airport Liaison %d tells Passenger to come to counter\n",myNumber,0,0);
      Print("AL %d Has alLineLock %d \n",myNumber,alLineLock,0);
      Print("AL %d signalling WaitingForAL_CV %d and alLineLock %d \n",myNumber, waitingForAL_C[myNumber],alLineLock);
      Signal(waitingForAL_C[myNumber], alLineLock);
    } else {
      /* Airport Liaison is not busy if there is no one in line */
      al_busy[myNumber] = 0;
    }
    
    /* Acquire the lock to the Airport Liaison
     We will use this lock to control the interactions between the Passenger
     and the Airport Liaison */
    Print("Acquiring the alLock %d\n",alLock[myNumber],0,0);
    /* alLock[myNumber]->Acquire(); */
    Acquire(alLock[myNumber]);
    
    /* After acquiring that lock, we release the Line Lock so who ever is waiting for the 
       Line Lock can then search for the shortest line and then get into the appropriate line */
    /* alLineLock.Release(); */
    Print("AL %d: Releasing the alLineLock %d \n",myNumber, alLineLock,0);
    Release(alLineLock);
    
    /* The Airport Liaison must now wait for the Passenger to go up to their counter 
     and give them their ticket 
     Sleeping the Airport Liaison frees up the alLock, wakes up one Passenger and puts them on the 
     Ready Queue */
    /* waitingForTicket_AL_C[myNumber]->Wait(alLock[myNumber]); */
    Print("AL %d : WaitingForTicketCV %d, alLock: %d\n",myNumber, waitingForTicket_AL_C[myNumber], alLock[myNumber]);
    Wait(waitingForTicket_AL_C[myNumber], alLock[myNumber]);
    
    /* Count the passenger's baggage */
    /*
    al_baggage_buffer[alPassenger[myNumber]] = passenger_baggage_buffer[alPassenger[myNumber]];
    
    Print("Airport Liaison %d takes note that Passenger %d has %d pieces of luggage\n",myNumber,alPassenger[myNumber],passenger_baggage_buffer[alPassenger[myNumber]]);
    */
  
    flight_number = pass_ticket_buffer[alPassenger[myNumber]].flight_number;   
    al_baggage_buffer[flight_number] += baggage_buffer[alPassenger[myNumber]].numberOfBags;
    /* printf("Flight %d has %d bags\n", flight_number,al_baggage_buffer[flight_number]);
     The Airport Liaison signals a Passenger, who is asleep waiting for the Airport
     Liaison to tell them where to go */
    /* waitingForTicket_AL_C[myNumber]->Signal(alLock[myNumber]); */
    Signal(waitingForTicket_AL_C[myNumber], alLock[myNumber]);
    pass_ticket_buffer[alPassenger[myNumber]].checkin_counter = pass_ticket_buffer[alPassenger[myNumber]].flight_number;  
    /* pass_ticket_buffer[alPassengerpmyNumber]].checkin_counter = */
    
    Print("Airport Liason %d directed passenger %d of airline %d\n", myNumber, alPassenger[myNumber], pass_ticket_buffer[alPassenger[myNumber]].flight_number);
    
    alPassengerCount++;
    /* alLock[myNumber]->Release(); */
    Release(alLock[myNumber]);
  }
  
}

/* Objects for Check In Staff */
/* Condition *waitingForCIS_C[numberOfCIS]; */
int waitingForCIS_C[numberOfCIS];
/*Condition *waitingForTicket_CIS_C[numberOfCIS]; */
int waitingForTicket_CIS_C[numberOfCIS];
/* Condition *onBreakCIS_C[numberOfCIS]; */
int onBreakCIS_C[numberOfCIS];
/* Condition *execLineCV[numberOfAirlines]; */
int execLineCV[numberOfAirlines];
/* Condition *waitingForExec_CIS_C[numberOfCIS]; */
int waitingForExec_CIS_C[numberOfCIS];

/* Lock *cisLineLock[numberOfAirlines]; */
int cisLineLock[numberOfAirlines];
/* Lock *cisLock[numberOfCIS]; */
int cisLock[numberOfCIS];
/* Lock *execLineLock[numberOfAirlines]; */
int execLineLock[numberOfAirlines];
/* Lock *execCISLock[numberOfCIS]; */
int execCISLock[numberOfCIS];

int cisLineLengths[numberOfCIS];
int cis_busy[numberOfCIS]; /* changed from bool */
int execLineLengths[numberOfAirlines];
int waitingForExec[numberOfCIS]; /* changed from bool */

/* Use this to keep track of passenger */
int cisPassenger[numberOfCIS];

int seatNumber[numberOfAirlines];

/*int cisPassengerCount = 0;*/
int cisBaggageWeight[numberOfAirlines]; /* keep track of the weight for each airline */

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

void Passenger() {
    int myLineNumber;

    int myFlightNumber;
    int amExecutive;

    int checkin_counter_number;
    int start, stop;
    int myNumber;
    
    Acquire(passengerNumLock);
    myNumber = passengerNum;
    passengerNum++;
    Release(passengerNumLock);

    /* --------------------------------------------------------
       1. Passenger goes to see Airport Liaison
       
       -------------------------------------------------------- */

    /* Passenger acquires the lock so they can search for shortest line amongst all lines */
    /* alLineLock.Acquire(); */
    Acquire(alLineLock);
    
    /* Declare the variable for the Passenger's line number
     We will reuse this variable for all */
    
    amExecutive = pass_ticket_buffer[myNumber].executive;
    /*if(amExecutive)
      printf("Passenger %d is an executive passenger\n",myNumber);*/
  
    /* Set the Passenger's Line number */
  
    myLineNumber = findShortestLine(alLineLengths,7);
    Print("Passenger %d chose Liaison %d with a line of length %d\n",myNumber,myLineNumber,alLineLengths[myLineNumber]);
    
    /* If there are people in the line, or the Airport Liaison is busy
       then the Passenger must wait in line and NOT approach the Airport Liaison */
    if((alLineLengths[myLineNumber] > 0)||(al_busy[myLineNumber]==1)) {
      alLineLengths[myLineNumber]++;
      /* waitingForAL_C[myLineNumber]->Wait(&alLineLock); */
      Print("Passenger %d waiting for Airport Liaison %d with alLineLock %d\n",myNumber,myLineNumber,alLineLock);
      Print("Passenger %d holding waitingForALCV %d\n",myNumber, waitingForAL_C[myLineNumber],0);
      Wait(waitingForAL_C[myLineNumber], alLineLock);
      Print("Passenger %d signalled \n",myNumber,0,0);
      al_busy[myLineNumber] = 1;
    }
    /* alLineLock.Release(); */
    Release(alLineLock);

    /* alLock[myLineNumber]->Acquire(); */
    Print("Passenger %d: Acquiring alLock %d\n",myNumber, alLock[myLineNumber],0);
    Acquire(alLock[myLineNumber]);
    alLineLengths[myLineNumber]--;
    /* Passenger is told to go to counter, and hands their ticket to Liaison */
    Print("Passenger %d goes to Airport Liaison %d\n",myNumber,myLineNumber,0);
    alPassenger[myLineNumber] = myNumber;
    /*waitingForTicket_AL_C[myLineNumber]->Signal(alLock[myLineNumber]);
      waitingForTicket_AL_C[myLineNumber]->Wait(alLock[myLineNumber]);*/
    Signal(waitingForTicket_AL_C[myLineNumber], alLock[myLineNumber]);
    Wait(waitingForTicket_AL_C[myLineNumber], alLock[myLineNumber]);
    
    /* alLock[myLineNumber]->Release(); */
    Release(alLock[myLineNumber]);

  /* --------------------------------------------------------
   2. Passenger goes to see Airport check in staff
  
  
   -------------------------------------------------------- */

  /* Acquire the Lock to the line
   Only use one lock for all 5 lines, because only one Passenger at 
   a time can be looking for the shortest line */
  checkin_counter_number = pass_ticket_buffer[myNumber].checkin_counter;
  

  /*int start, stop;*/
  start = (pass_ticket_buffer[myNumber].checkin_counter)*(numberOfCIS/3);
  stop  = start + (numberOfCIS/3) - 1;
  /* Figure out which */
  
  if(amExecutive) {
    int tempCount = 0;
    /* execLineLock[checkin_counter_number]->Acquire(); */
    Acquire(execLineLock[checkin_counter_number]);

    execLineLengths[checkin_counter_number]++;
    /* execLineCV[checkin_counter_number]->Wait(execLineLock[checkin_counter_number]); */
    Print("Passenger %d of Airline %d is waiting in the executive class line\n",myNumber,0,0);
    Wait(execLineCV[checkin_counter_number], execLineLock[checkin_counter_number]);
    if(current_test == 3) {
      /* currentThread->Finish(); */
      Exit(0);
    }
    /* Find the line of the Check in staff */
    for(tempCount = start; tempCount <= stop; tempCount++) {
      if(waitingForExec[tempCount] == 1) {
        myLineNumber = tempCount;
        break;
      }
    }
    
    /* Tell CIS that passenger is ready */
    /* execLineCV[checkin_counter_number]->Signal(execLineLock[checkin_counter_number]);
     */
     
    /*Waiting for CIS to give them boarding pass
     execLineCV[checkin_counter_number]->Wait(execLineLock[checkin_counter_number]); */
    execLineLengths[checkin_counter_number]--;
  
    /* execCISLock[myLineNumber]->Acquire(); */
    Acquire(execCISLock[myLineNumber]);
    cisPassenger[myLineNumber] = myNumber;
    /* execLineLock[checkin_counter_number]->Release(); */
    Release(execLineLock[checkin_counter_number]);
    
    /*waitingForExec_CIS_C[myLineNumber]->Signal(execCISLock[myLineNumber]);
      waitingForExec_CIS_C[myLineNumber]->Wait(execCISLock[myLineNumber]);*/
    Signal(waitingForExec_CIS_C[myLineNumber], execCISLock[myLineNumber]);
    Wait(waitingForExec_CIS_C[myLineNumber], execCISLock[myLineNumber]);

    /* execCISLock[myLineNumber]->Release(); */
    Release(execCISLock[myLineNumber]);
  
  } else {

    /* cisLineLock[checkin_counter_number]->Acquire(); */
    Acquire(cisLineLock[checkin_counter_number]);

    /* Set the Passenger's line number */
    myLineNumber = findCISShortestLine(cisLineLengths,start,stop);
 
    Print("Passenger %d of Airline chose Airline Check-In staff %d with a line length %d\n",myNumber,myLineNumber,cisLineLengths[myLineNumber]);

    cisLineLengths[myLineNumber]++;
    /* onBreakCIS_C[myLineNumber]->Signal(cisLineLock[checkin_counter_number]); */
    Signal(onBreakCIS_C[myLineNumber], cisLineLock[checkin_counter_number]);

    cis_current_passenger_serving[myLineNumber] = myNumber;
    
    /* waitingForCIS_C[myLineNumber]->Wait(cisLineLock[checkin_counter_number]);*/
    Wait(waitingForCIS_C[myLineNumber], cisLineLock[checkin_counter_number]);
    if(current_test == 3) {
      /* currentThread->Finish(); */
      Exit(0);
    }
    /* cisLineLengths[myLineNumber]--; */
    
    /* printf("%s going to see Airline Check In Staff %d\n",currentThread->getName(), myLineNumber);  */
    /* cisLock[myLineNumber]->Acquire(); */
    Acquire(cisLock[myLineNumber]);

    cisPassenger[myLineNumber]=myNumber;

    cisLineLengths[myLineNumber]--;
    /* cisLineLock[checkin_counter_number]->Release(); */
    Release(cisLineLock[checkin_counter_number]);
    /* cisLock[myLineNumber]->Acquire(); */
    
    
    /* The Passenger now has the line number, so they should go to sleep and
     release the line lock, letting another Passenger search for a line
     printf("%s giving airline ticket to Airline Check In Staff %d\n", currentThread->getName(), myLineNumber); */
    /* waitingForTicket_CIS_C[myLineNumber]->Signal(cisLock[myLineNumber]);
       waitingForTicket_CIS_C[myLineNumber]->Wait(cisLock[myLineNumber]); */
    Signal(waitingForTicket_CIS_C[myLineNumber], cisLock[myLineNumber]);
    Wait(waitingForTicket_CIS_C[myLineNumber], cisLock[myLineNumber]);
    /* cisLock[myLineNumber]->Release(); */
    Release(cisLock[myLineNumber]);


  }
  Print("Passenger %d of Airline %d was informed to board at gate %d\n",myNumber,pass_ticket_buffer[myNumber].flight_number,0);

  /* --------------------------------------------------------
   3. Passenger goes to see Airport Security Officer
  

   
   -------------------------------------------------------- */
  
  /* soLineLock.Acquire(); */
  Acquire(soLineLock);  

  myLineNumber = findShortestLine(soLineLengths,7);
  soLineLengths[myLineNumber]++;

  /* waitingSO_C[myLineNumber]->Signal(&soLineLock); */
  Signal(waitingSO_C[myLineNumber], soLineLock);
  /*soPassenger[myLineNumber] = myNumber;*/
  /*
    printf("Passenger %d gives the hand-luggage to screening officer %d\n", myNumber, myLineNumber);*/
  /* waitingForSO_C[myLineNumber]->Signal(&soLineLock); */
  /* waitingForSO_C[myLineNumber]->Wait(&soLineLock); */

  Wait(waitingForSO_C[myLineNumber], soLineLock);
    /* soLineLock.Release(); */
  soLineLengths[myLineNumber]--;
  Release(soLineLock);
  /* soLock[myLineNumber]->Acquire(); */
  Acquire(soLock[myLineNumber]);

  soPassenger[myLineNumber] = myNumber;

  /* The Passenger now has the line number, so they should go to sleep and
     release the line lock, letting another Passenger search for a line */
  /* waitingForTicket_SO_C[myLineNumber]->Signal(soLock[myLineNumber]); */
  Signal(waitingForTicket_SO_C[myLineNumber], soLock[myLineNumber]);
  /* printf("Passenger %d gives the hand-luggage to screening officer %d\n", myNumber, myLineNumber); */
  /* waitingForTicket_SO_C[myLineNumber]->Wait(soLock[myLineNumber]); */
  Wait(waitingForTicket_SO_C[myLineNumber], soLock[myLineNumber]);

  /* soLock[myLineNumber]->Release(); */
  Release(soLock[myLineNumber]);

  /* --------------------------------------------------------
   4. Passenger goes to see Airport Security Inspector
  
  
   -------------------------------------------------------- */
  Acquire(siLineLock);
  
  myLineNumber = passengerGoToSI[myNumber];

  /*waitingSI_C[myLineNumber]->Signal(&siLineLock);*/
  Print("Passenger %d before signaling waitingsic\n",myNumber,0,0);
  Signal(waitingSI_C[myLineNumber], siLineLock);
  siLineLengths[myLineNumber]++;
  /* soPassenger[myLineNumber] = myNumber; */
  /* waitingForSO_C[myLineNumber]->Signal(&soLineLock); */
  /* waitingForSI_C[myLineNumber]->Wait(&siLineLock); */
  Print("Passenger %d before waiting waitingsic\n",myNumber,0,0);  
  Wait(waitingForSI_C[myLineNumber], siLineLock);
  siLineLengths[myLineNumber]--;
  Release(siLineLock);
  /* siLineLock.Release(); */

  /* siLock[myLineNumber]->Acquire(); */
  Acquire(siLock[myLineNumber]);
  siPassenger[myLineNumber] = myNumber;

  /* The Passenger now has the line number, so they should go to sleep and
     release the line lock, letting another Passenger search for a line */
  /*waitingForTicket_SI_C[myLineNumber]->Signal(siLock[myLineNumber]);*/
  Signal(waitingForTicket_SI_C[myLineNumber], siLock[myLineNumber]);

  /*waitingForTicket_SI_C[myLineNumber]->Wait(siLock[myLineNumber]);*/
  Wait(waitingForTicket_SI_C[myLineNumber], siLock[myLineNumber]);

  /* siLock[myLineNumber]->Release(); */
  Release(siLock[myLineNumber]);
  

  /* --------------------------------------------------------
   5. Passenger goes to boarding lounge
  
  
   -------------------------------------------------------- */
  myFlightNumber = boarding_pass_buffer[myNumber].flight_number;
  if(myFlightNumber == 0) {
    Print("=============Passenger %d acquring airline lock\n",myNumber,0,0);
  }
  /*airlineLock[myFlightNumber]->Acquire();*/
  Acquire(airlineLock[myFlightNumber]);
  flightCount[myFlightNumber]++;
  /*waitingForCallAM_C[myFlightNumber]->Wait(airlineLock[myFlightNumber]);*/
  if(myFlightNumber == 0) {
    Print("---------------Passenger %d waiting at gate %d\n",myNumber,myFlightNumber,0);
  }
  Wait(waitingForCallAM_C[myFlightNumber], airlineLock[myFlightNumber]);
  /*airlineLock[myFlightNumber]->Release();*/
  Release(airlineLock[myFlightNumber]);
    
  Print("------------------------------------------------------------Passenger %d boarded Airline %d \n",myNumber,myFlightNumber,0);
  Exit(0);
    /* FIN */
    
}

int main () {

  int i, b;
  int randNumBags, randNumWeight;
  /* Thread *t; Create a thread pointer variable */
  char *name;
  /*
   * Needs Airlines
   * Bags and weights
   *
   */

  /* printf("Starting Airport Simulation\n"); */


  passengerNumLock = CreateLock(1);
  Print("passenger number lock %d\n",passengerNumLock,0,0);
  airportLNumLock  = CreateLock(1);
  cargoHandlerNumLock = CreateLock(1);
  securityOffNumLock  = CreateLock(1);
  securityInsNumLock  = CreateLock(1);
  cisNumLock          = CreateLock(1);
 
  passengerNum = 0;
  airportLNum = 0;
  cargoHandlerNum = 0;
  securityOffNum = 0;
  securityInsNum = 0;
  cisNumLock     = 0;
  
  /* -------------------------------------------------
     Initialize Condition Variables */

  /* goToSleep condition variable */
  goToSleep = CreateCondition();

  /* onBreakCH condition variable */
  onBreakCH = CreateCondition();
  
  /* waitingForAL condition variable */
  for(i = 0; i < numberOfAL; i++) {
    /*name = new char [20];
    sprintf(name, "WFAL_C%d",i);
    waitingForAL_C[i] = new Condition(name);*/
    waitingForAL_C[i] = CreateCondition();
    al_busy[i] = 1;
  }

  /* waitingForCIS condition variable */
  for(i = 0; i < numberOfCIS; i++) {
    /*name = new char [20];
    sprintf(name,"WFCIS_C%d",i);
    waitingForCIS_C[i] = new Condition(name);*/
    waitingForCIS_C[i] = CreateCondition();
    cis_busy[i] = 1;
  }

  /* waitingForSO condition variable */
  for(i = 0; i < numberOfSO; i++) {
    /*name = new char [20];
    sprintf(name,"WFSO_C%d",i);
    waitingForSO_C[i] = new Condition(name);*/
    waitingForSO_C[i] = CreateCondition();
    so_busy[i] = 1;
  }

  /* waitingForSI condition variable */
  for(i = 0; i < numberOfSO; i++) {
    /*name = new char [20];
    sprintf(name,"WFSI_C%d",i);
    waitingForSI_C[i] = new Condition(name);*/
    waitingForSI_C[i] = CreateCondition();
    si_busy[i] = 1;
  }

  /* waitingForTicket_AL condition variable */

  for(i = 0; i < numberOfAL; i++) {
    /*name = new char [20];
    sprintf(name, "WFTICKET_AL_C%d",i);
    waitingForTicket_AL_C[i] = new Condition(name);*/
    waitingForTicket_AL_C[i] = CreateCondition();
  }
  
  /* waitingForTicket_CIS_C condition variable */

  for(i = 0; i < numberOfCIS; i++) {
    /* name = new char[20];
    sprintf(name,"CISTICKET_C%d",i);
    waitingForTicket_CIS_C[i] = new Condition(name); */
    waitingForTicket_CIS_C[i] = CreateCondition();
  }

  /* waitingForTicket_SO_C condition variable */

  for(i = 0; i < numberOfSO; i++) {
    /*name = new char[20];
    sprintf(name,"SOTICKET_C%d",i);
    waitingForTicket_SO_C[i] = new Condition(name);*/
    waitingForTicket_SO_C[i] = CreateCondition();
  }

  /* waitingForTicket_SI_C condition variable */

  for(i = 0; i < numberOfSO; i++) {
    /*name = new char[20];
    sprintf(name,"SITICKET_C%d",i);
    waitingForTicket_SI_C[i] = new Condition(name);*/
    waitingForTicket_SI_C[i] = CreateCondition();
  }

  /* onBreakCIS_C condition variable */

  for(i = 0; i < numberOfCIS; i++) {
    /*name = new char[20];
    sprintf(name, "CISBREAK_C%d",i);
    onBreakCIS_C[i] = new Condition(name);*/
    onBreakCIS_C[i] = CreateCondition();
  }
   
  /* waitingForCallAM_C condition variable */

  for(i = 0; i < numberOfAirlines; i++) {
    /*name = new char[20];*/
    /*sprintf(name, "AM_C%d",i);*/
    /*waitingForCallAM_C[i] = new Condition(name);*/
    waitingForCallAM_C[i] = CreateCondition();
  }


  /* ---------------------------
     Initialize conveyorBelt lock
  */
  conveyorBelt_Lock = CreateLock(1);

  /* ---------------------------
     Initialize siLineLock
  */
  siLineLock = CreateLock(1);
  soLineLock = CreateLock(1);
  /* ---------------------------
     Initialize alLineLock
  */
  alLineLock = CreateLock(1);

  /*--------------------------------------------------

   -------------------------------------------------
   Initialize Airport Liaison Locks
   printf("creating al locks\n"); */


  for(i = 0; i < numberOfAL; i++) {
    /*name = new char[20];
    sprintf(name,"alLock%d",i);
    alLock[i] = new Lock(name);*/
    alLock[i] = CreateLock(1);
  }
  /* -------------------------------------------------

   -------------------------------------------------
   Initialize Check in Staff Line Locks
   printf("creating al locks\n"); */
  for(i = 0; i < numberOfAirlines; i++) {
    /*name = new char[20];
    sprintf(name,"cisLineLock%d",i);
    cisLineLock[i] = new Lock(name);*/
    cisLineLock[i] = CreateLock(1);
  }

  for(i=0; i < numberOfAirlines; i++) {
    /*name = new char[20];
    sprintf(name,"execLineLock%d",i);
    execLineLock[i] = new Lock(name); */
    execLineLock[i] = CreateLock(1);
    /* name = new char [20];
    sprintf(name,"execLineCV%d",i);
    execLineCV[i]  = new Condition(name);*/
    execLineCV[i] = CreateCondition();
  }
  /* -------------------------------------------------

   -------------------------------------------------
   Initialize Airline check in staff Locks
   printf("creating al locks\n"); */
  for(i = 0; i < numberOfCIS; i++) {
    /* name = new char[20];
    sprintf(name,"cisLock%d",i);
    cisLock[i] = new Lock(name); */
    cisLock[i] = CreateLock(1);
    /*name = new char[20];
    sprintf(name,"cisExecLock%d",i);
    execCISLock[i] = new Lock(name);*/
    execCISLock[i] = CreateLock(1);
    /*name = new char[20];
    sprintf(name,"waitingForExec_CIS_C%d",i);
    waitingForExec_CIS_C[i] = new Condition(name);*/
    waitingForExec_CIS_C[i] = CreateCondition();
  }
  /* -------------------------------------------------

   -------------------------------------------------
   Initialize Security Officer check in staff Locks
   printf("creating al locks\n"); */
  for(i = 0; i < numberOfSO; i++) {
    /*name = new char[20];
    sprintf(name,"soLock%d",i);
    soLock[i] = new Lock(name);*/
    soLock[i] = CreateLock(1);
    
    /*name = new char[20];
    sprintf(name,"waitingsoc%d",i);
    waitingSO_C[i] = new Condition(name);*/
    waitingSO_C[i] = CreateCondition();
    
    /*name = new char[20];
    sprintf(name,"waitingsic%d",i);
    waitingSI_C[i] = new Condition(name);*/
    waitingSI_C[i] = CreateCondition();
    
    /*name = new char[20];
    sprintf(name,"returnlinecv%d",i);
    returnLineCV[i] = new Condition(name);*/
    returnLineCV[i] = CreateCondition();
    
    /*name = new char[20];
    sprintf(name,"waitreturnlinecv%d",i);
    waitingForReturn_SI_C[i] = new Condition(name);*/
    waitingForReturn_SI_C[i] = CreateCondition();

    /*name = new char[20];
    sprintf(name,"sireturnlock%d",i);
    siReturnLock[i] = new Lock(name);*/
    siReturnLock[i] = CreateLock(1);

    /*name = new char[20];
    sprintf(name,"sirlock%d",i);
    siRLock[i] = new Lock(name);*/
    siRLock[i] = CreateLock(1);

    siLineReturns[i]=0;


  }

  for(i = 0; i < numberOfSO; i++) {
    /*name = new char[20];
    sprintf(name,"siLock%d",i);
    siLock[i] = new Lock(name);*/
    siLock[i] = CreateLock(1);
  }
  /* -------------------------------------------------
   -------------------------------------------------
   Initialize Security Officer check in staff Locks
   printf("creating al locks\n"); */
  for(i = 0; i < numberOfAirlines; i++) {
    /*name = new char[20];*/
    /*sprintf(name,"airLineLock%d",i);*/
    /*airlineLock[i] = new Lock(name);*/
    airlineLock[i] = CreateLock(1);
  }




  /* -------------------------------------------------

   -------------------------------------------------
   Initialize the Line Lengths 

   Line length for Airport Liaison */
  for( i = 0; i < numberOfAL; i++) {
    alLineLengths[i] = 0;
  }

  /* Line length for Airline check in staff */
  for( i = 0; i < numberOfCIS; i++) {
    cisLineLengths[i] = 0;
    cis_current_passenger_serving[i] = 0;
  }

  /* Line length for screening officers */
  for( i = 0; i < numberOfSO; i++) {
    soLineLengths[i] = 0;
  }

  /* Line length for security inspectors */
  for( i = 0; i < numberOfSO; i++) {
    siLineLengths[i] = 0;
  }
  for(i=0; i < numberOfAirlines; i++) {
    execLineLengths[i] = 0;
  }
  /* ------------------------------------------------- */


  for(i=0; i < numberOfAirlines; i++) {
    flightCount[i]=0;
    cisFlightCount[i]=0;
    siAirlineCount[i]=0;
  }

  for(i = 0; i < numberOfAirlines; i++) {
    alreadyCalled[i] = 0; 
  }

  /* Initialize the baggage buffer */
  for(i = 0; i < numberOfAirlines; i++) {
    al_baggage_buffer[i]        = 0;
    cis_baggage_buffer[i]       = 0;
    cargoHandlerBaggageCount[i] = 0;
    seatNumber[i]               = 0;
  }

  /* int randNumBags,randNumWeight; */
  /* Create the 20 passenger for our airport simulation */
  /*printf("Creating Passengers\n");*/
  for( i=0; i < numberOfPassengers; i++) {
    /* Create a ticket for the passenger */
    pass_ticket_buffer[i].passenger_number = i;
    pass_ticket_buffer[i].flight_number = (i%numberOfAirlines);
    if((i%8)==0) {
      pass_ticket_buffer[i].executive = 1;
    } else {
      pass_ticket_buffer[i].executive = 0;
    }
    pass_ticket_buffer[i].checkin_counter = -1;
    /* TO DO 
       Randomize weights */

    /* int randNumBags, randNumWeight; */
    randNumBags = 2; /*random # between 2-3 */
    /*printf("DEBUG: randNumBags=%d\n",randNumBags); */
    baggage_buffer[i].numberOfBags = randNumBags;
    numBagsDuringSetup[ pass_ticket_buffer[i].flight_number ] += randNumBags;
    baggage_buffer[i].weight = 0;

    for(b=0; b<randNumBags; b++) {
      randNumWeight = 30;  /* random # between 30-59 */
      baggage_buffer[i].weight += randNumWeight;
      bagWeightsDuringSetup[ pass_ticket_buffer[i].flight_number ] += randNumWeight;
      baggage_buffer[i].weights[b] = randNumWeight;
      totalweight += randNumWeight;
    }

    /*
    if(i%2==0) {
      baggage_buffer[i].numberOfBags = 2;
      baggage_buffer[i].weight = 120;
      totalweight += 120;
      numBagsDuringSetup[ pass_ticket_buffer[i].flight_number ] +=2;
      bagWeightsDuringSetup[ pass_ticket_buffer[i].flight_number ] += 120;
    } else {
      baggage_buffer[i].numberOfBags = 3;
      baggage_buffer[i].weight = 180;
      totalweight += 180;
      numBagsDuringSetup[ pass_ticket_buffer[i].flight_number ] += 3;
      bagWeightsDuringSetup[ pass_ticket_buffer[i].flight_number ] += 180;
    }
    */

    baggage_buffer[i].passenger_number = i;
    baggage_buffer[i].airline_code = (i%numberOfAirlines);
    boarding_pass_buffer[i].passenger_number = i;
    boarding_pass_buffer[i].flight_number = (i%numberOfAirlines);
    boarding_pass_buffer[i].seat_number = -1;

    /* Initialize so pass or fail */
    so_passOrFail[i] = 1;

    /*name = new char [20]; 
    sprintf(name,"Passenger%d",i);
    /* printf("Creating %s\n",name); */
    /*t = new Thread(name); // Give the Passenger a name */
    Fork(Passenger,1);
  }

  /* Create all the Airport Staff First */

  /* Create the Airport Liaison */
  
  for(i = 0; i < numberOfAL; i++) {
    /*
    name = new char[20];
    sprintf(name, "AL%d",i);
    t = new Thread(name);
    */
    Fork(AirportLiaison,2);
  }

  /* Create the Airline Check In Staff */

  for(i=0; i < numberOfCIS; i++) {
    waitingForExec[i] = 0;
    /*
    name = new char[20];
    sprintf(name, "Airline check-in-staff%d",i);
    t = new Thread(name);
    */
    Fork(CheckInStaff,3);
  }
  
  /* Create the Security Officer Staff */

  for(i=0; i < numberOfSO; i++) {
    /*
    name = new char[20];
    sprintf(name, "SecurityOfficer%d",i);
    t = new Thread(name);
    */
    Fork(SecurityOfficer,4);
  }
  
  /* Create the Airline Check In Staff */

  for(i=0; i < numberOfSO; i++) {
    /*
    name = new char[20];
    sprintf(name, "SecurityInspector%d",i);
    t = new Thread(name);
    */
    Fork(SecurityInspector,5);
  }

  for(i=0; i < numberOfCH; i++) {
    /*
    name = new char[20];
    sprintf(name, "CargoHandler%d",i);
    t = new Thread(name);
    */
    Fork(CargoHandler,6);
  }
  /*
    name = new char[20];
    name = "AirportManager";
    t = new Thread(name);
  */

  Fork(AirportManager,7);
  
  if(current_test == 0) {
    /*
  printf("Number of airport liasons = %d\n",numberOfAL);
  printf("Number of airlines = %d\n",numberOfAirlines);
  printf("Number of check-in staff = %d\n",numberOfCIS);
  printf("Number of cargo handlers = %d\n",numberOfCH);
  printf("Number of screening officers = %d\n",numberOfSO);
  printf("Total number of passengers = %d\n",numberOfPassengers);
    */
  for(i=0; i < numberOfAirlines; i++) {
    int j;
    int numPassengersOnAirline = 0;
    for(j=0; j < numberOfPassengers; j++) {
      if(pass_ticket_buffer[j].flight_number == i)
        numPassengersOnAirline++;
    }
    /*printf("Number of passengers for airline %d = %d\n",0,numPassengersOnAirline);*/
  }

  for(i=0; i < numberOfPassengers; i++) {
    int j;
    /*
      printf("Passenger %d belongs to airline %d\n",i,pass_ticket_buffer[i].flight_number);
    printf("Passenger %d: Number of bags = %d\n",i,baggage_buffer[i].numberOfBags);
    printf("Passenger %d: Weight of bags = ",i);
    */
    for(j = 0; j < baggage_buffer[i].numberOfBags; j++) {
      if( j!=0 && j!= (baggage_buffer[i].numberOfBags)) {
        /*printf(",");
          printf("%d",baggage_buffer[i].weights[j]);
        */
      }
    }
    /*printf("\n");*/
  }

  for(i=0; i < numberOfCIS; i++) {
    /*printf("Airline check-in staff %d belongs to airline %d\n",i,i);*/
  }
  }

}

void Test1() {
  current_test = 1;
}

void Test2() {
  current_test = 2;
}

void Test3() {
  current_test = 3;
}

void Test4() {
  current_test = 4;
}

void Test5() {
  current_test = 5;
}

void Test6() {
  int i;
  char* name;
  onBreak_CH = 0;
  current_test = 6;
  for(i=0; i <numberOfPassengers; i++) {
    conveyorBelt[i].number_of_bags = (i%2)+2;
    conveyorBelt[i].airline_code = (i%3);
    conveyorBelt[i].weight = 60;
  }
  for(i=0; i < numberOfCH; i++) {
    /*
    name = new char[20];
    sprintf(name, "CargoHandler%d",i);
    t = new Thread(name);
    */    
    /*Fork(CargoHandler);*/
  }
}

void Test7() {
  current_test = 7;
}

void Test8() {
  current_test = 8;
}

void Test9() {
  /* current_test = 9; */
}

void Test10() {
  current_test = 10;
}
/*
void Main() {
  int number_run;
  cout << "Enter a test number" << endl;
  cout << "0. Entire Simulation" << endl;
  cout << "1. Passenger getting into shortest line" << endl;
  cout << "2. Passenger directed to correct airline counters" << endl;
  cout << "3. Economy passenger gets into shortest line; Executive goes to executive line" << endl;
  cout << "4. Executives are given priority of line" << endl;
  cout << "5. Screening officer chooses available Security Officer" << endl;
  cout << "6. Cargo handlers choose bag from conveyor belt" << endl;
  cout << "7. Passengers hand over luggage to screening officer" << endl;
  cout << "8. Passenger returns to same security inspector after questioning" << endl;
  cout << "9. Baggage weights are equal on all counts" << endl;
  cout << "10. Passenger hands over boarding pass to Security Inspector" << endl;

  cin >> number_run;
  if(number_run == 0)
    AirportSimulation();
  if(number_run == 1)
    Test1();
  if(number_run == 2)
    Test2();
  if(number_run == 3)
    Test3();
  if(number_run == 4)
    Test4();
  if(number_run == 5)
    Test5();
  if(number_run == 6)
    Test6();
  if(number_run == 7)
    Test7();
  if(number_run == 8)
    Test8();
  if(number_run == 9)
    Test9();
  if(number_run == 10)
    Test10();
  
}
*/
