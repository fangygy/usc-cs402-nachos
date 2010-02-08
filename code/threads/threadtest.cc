// threadtest.cc 
//      Simple test case for the threads assignment.
//
//      Create two threads, and have them context switch
//      back and forth between themselves by calling Thread::Yield, 
//      to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

// Authors Alex Lee

#include "copyright.h"
#include "system.h"
#include "functions.h"
// #include "testsuite.h"
#ifdef CHANGED
#include "synch.h"
#endif

using namespace std;
// ---------------------------------------------------------------------
/*
 * PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO PART TWO 
 * RUN AIRPORT SIMULATIONS
 *
 *
 *
*/
// ---------------------------------------------------------------------

// Global variables

#define numberOfPassengers 20
#define numberOfAL 7
#define numberOfCIS 15
#define numberOfSO 7
#define numberOfAirlines 3
#define numberOfCH 6

#define probabilityPassingSO 90
#define probabilityPassingSI 90

int current_test = 0;

int sicount = 1;
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
  bool executive;
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
} baggage_buffer[numberOfPassengers];

// int passenger_baggage_buffer[numberOfPassengers];
// int cis_baggage_buffer[numberOfPassengers];

int al_baggage_buffer[numberOfAirlines];
int cis_baggage_buffer[numberOfAirlines];
int al_current_passenger_serving[numberOfAL]; // must be equal to the number of airport liaisons
int cis_current_passenger_serving[numberOfCIS];

Condition *waitingForCallAM_C[numberOfAirlines];
Lock *airlineLock[numberOfAirlines];
int flightCount[numberOfAirlines];
int cisFlightCount[numberOfAirlines];
bool alreadyCalled[numberOfAirlines];
Condition goToSleep("goToSleep");

int totalweight = 0; // for debugging

Condition onBreakCH("ch_cv");
Lock conveyorBelt_Lock("cb_lock");
Lock *airline_CH_Lock[numberOfAirlines];
bool onBreak_CH = true;

struct conveyorBelt {
  int number_of_bags;
  int weight;
  int airline_code;
} conveyorBelt[numberOfPassengers];

int cargoHandlerBaggageCount[numberOfAirlines];

bool passengersFailedSI[numberOfPassengers];

int passengerGoToSI[numberOfPassengers];
int g = 0;

void AirportManager(int myNumber) {
  if(current_test > 0) {
    currentThread->Finish();
  }
  while(true) {
    // if all passengers are accounted for
    // issue broadcast
    
    if(g > 30) {
      currentThread->Finish();
    }
    g++;
    conveyorBelt_Lock.Acquire();
    if(onBreak_CH) {
      for(int i = 0; i < numberOfPassengers; i++) {
        if(conveyorBelt[i].number_of_bags == 0) {
          // check next bag
        } else {
          printf("Airport Manager calls back all the cargo handlers from break\n");
          onBreakCH.Broadcast(&conveyorBelt_Lock);
          break;
        }
      }
    }
    conveyorBelt_Lock.Release();

    for(int i = 0; i < numberOfAirlines; i++) {
      airlineLock[i]->Acquire();
      if(alreadyCalled[0]&&alreadyCalled[1]&&alreadyCalled[2]) {
        // TODO
        // print out statistics

        printf("Passenger count reported by airport liason = %d\n",alPassengerCount);
        printf("Passenger count reported by airline check-in staff = %d\n",cisPassengerCount);
        printf("Passenger count reported by security inspector = %d\n",sicount);

        //for(int g = 0; g < numberOfPassengers; g++) {
        //  printf("[%d, %d]",g,conveyorBelt[g].number_of_bags);
        //}
        //printf("\n");
        for(int g = 0; g < numberOfAirlines; g++) {
          //printf("----------------Statistics--------------\n");
          printf("From setup: Baggage count of airline %d = %d\n",g,numBagsDuringSetup[g]);
          printf("From airport liason: Baggage count of airline %d = %d\n",g, al_baggage_buffer[g]);
          printf("From cargo handlers: Baggage count of airline %d = %d\n",g, cargoHandlerBaggageCount[g]);
          printf("From setup: Baggage weight of airline %d = %d\n",g,bagWeightsDuringSetup[g]);
          printf("From airline check-in staff: Baggage weight of airline %d = %d\n",g, cis_baggage_buffer[g]);
          printf("From cargo handlers: Baggage weight of airline %d = %d\n",g,chBagWeights[g]);
        }
        goToSleep.Wait(airlineLock[i]);
      }
      printf("flight %d count %d , cisflightcount %d\n",i,flightCount[i],cisFlightCount[i]); 
      if(!alreadyCalled[i]&&(flightCount[i] == cisFlightCount[i])&&(flightCount[i]!=0)&&(cisFlightCount[i]!=0)&&(cargoHandlerBaggageCount[i]==al_baggage_buffer[i])) {
        printf("Airport Manager gives a boarding call to airline %d\n",i);
        waitingForCallAM_C[i]->Broadcast(airlineLock[i]);
        alreadyCalled[i] = true;
      } else {
        
      }
      airlineLock[i]->Release();
    }
    for(int i = 0; i < 10; i++) {
      currentThread->Yield();
    }
  }
}

void CargoHandler(int myNumber) {
  
  if((current_test != 6)&&(current_test!=0)) {
    currentThread->Finish();
  }
  
  while(true) {
    conveyorBelt_Lock.Acquire();
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
    
    if(onBreak_CH) {
      printf("Cargo Handler %d is going on a break\n",myNumber);
      if(current_test == 6) {
        conveyorBelt_Lock.Release();
        currentThread->Finish();
      } else {
        onBreakCH.Wait(&conveyorBelt_Lock);
	printf("Cargo Handler %d returned from break\n",myNumber);
      }
    }
    
    for(int i = 0; i < numberOfPassengers; i++) {
      if(conveyorBelt[i].number_of_bags > 0) {
        // Cargo Handler Found a bag
        printf("Cargo Handler %d picked bag of airline %d with weighing %d lbs\n",myNumber,conveyorBelt[i].airline_code,conveyorBelt[i].weight);
        chBagWeights[conveyorBelt[i].airline_code] += conveyorBelt[i].weight;
        cargoHandlerBaggageCount[conveyorBelt[i].airline_code]+=conveyorBelt[i].number_of_bags;
        conveyorBelt[i].number_of_bags = 0;
        conveyorBelt[i].airline_code = -1;
        conveyorBelt_Lock.Release();
        break;
      }
      // Goes through entire array and can not find a single bag
      if(i == (numberOfPassengers-1)) {
	// Goes on break
        onBreak_CH = true;
        onBreakCH.Wait(&conveyorBelt_Lock);
        // conveyorBelt_Lock.Release();
      }      
    }
    // onBreak_CH = true;
    conveyorBelt_Lock.Release();

  }
}

Condition *waitingForSI_C[numberOfSO];
Condition *waitingForTicket_SI_C[numberOfSO];
//Condition *waitingForSIAfterQuestioning_C[numberOfSO];
//int siBackFromQuestioningLineLengths[numberOfSO];
Lock siLineLock("si_LL");
Lock *siLock[numberOfSO];
Lock siAirplaneCountLock("si_ALL");

int siLineLengths[numberOfSO];
bool si_busy[numberOfSO];
bool so_passOrFail[numberOfSO];
int siPassenger[numberOfSO];
int siAirlineCount[numberOfAirlines];

void SecurityInspector(int myNumber) {
  if((current_test == 1)||(current_test==2)||(current_test==3)||(current_test==4)||(current_test==6)) {
    currentThread->Finish();
  }
  while(true) {
    
    si_busy[myNumber] = false;
    
    bool passengerQuestioned = false;
    
    siLineLock.Acquire();

    // Security Inspectors checks to make sure if some one is returning back from questioning
    if(siLineLengths[myNumber]>0) {
      //printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSI_C[myNumber]->Signal(&siLineLock);
    } else {
      waitingForSI_C[myNumber]->Wait(&siLineLock);
      //printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSI_C[myNumber]->Signal(&siLineLock);
    }

    si_busy[myNumber] = true;
    
    siLock[myNumber]->Acquire();
    siLineLock.Release();

    /*
    if(passengersFailedSI[ siPassenger[myNumber] ]) {
      // passenger returning from further questioning
      passengerQuestioned = true;
    }
    */
    waitingForTicket_SI_C[myNumber]->Wait(siLock[myNumber]);
    waitingForTicket_SI_C[myNumber]->Signal(siLock[myNumber]);
    /*
    if( !(passengerQuestioned) ) {
      bool passedSI;
      int randomNum = rand() % 100;
      if(randomNum < probabilityPassingSI) {
        //passenger passed SI
        passedSI = true;
        
      } else {
        //passenger failed SI
        passedSI = false;
      }

      if(!passedSI | !so_passOrFail[myNumber]) {
        //passenger failed one or more inspections, raise suspicion
        printf("Security inspector %d asks passenger %d to go for further examination\n", myNumber, siPassenger[myNumber]);
        passengersFailedSI[ siPassenger[myNumber] ] = true;

      } else {
        // Clear passenger and direct to Boarding
        sicount++;
        printf("Security inspector %d allows passenger %d to board\n", myNumber,siPassenger[myNumber]);
      }
    } else {
      // Passenger returned from further questioning
      printf("Security inspector %d permits returning passenger %d to board\n", myNumber, siPassenger[myNumber]);
      // Clear passenger and direct to boarding
      sicount++;
      //printf("%s: moving Passenger %d to Boarding: \n", currentThread->getName(),siPassenger[myNumber], sicount);
    }
    */
    sicount++;
    printf("%s: moving Passenger %d to Boarding: \n", currentThread->getName(),siPassenger[myNumber], sicount);
    printf("si has moved %d passengers\n",sicount);
    // Keep track of how many passengers are cleared for each airline
    siAirplaneCountLock.Acquire();
    siAirlineCount[boarding_pass_buffer[siPassenger[myNumber]].flight_number]++;
    siAirplaneCountLock.Release();
    siLock[myNumber]->Release();

  }
}

Condition *waitingForSO_C[numberOfSO];
Condition *waitingForTicket_SO_C[numberOfSO];
Lock soLineLock("sl_LL");
Lock *soLock[numberOfSO];
int soLineLengths[numberOfSO];
bool so_busy[numberOfSO];
int soPassenger[numberOfSO];

void SecurityOfficer(int myNumber) {
  if((current_test == 1)||(current_test==2)||(current_test==3)||(current_test==4)||(current_test==6)) {
    currentThread->Finish();
  }
  while(true) {
    
    soLineLock.Acquire();
    
    if(soLineLengths[myNumber]>0) {
      //printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSO_C[myNumber]->Signal(&soLineLock);
    } else {
      waitingForSO_C[myNumber]->Wait(&soLineLock);
      //printf("%s: Telling passenger to come through Security\n", currentThread->getName());
      waitingForSO_C[myNumber]->Signal(&soLineLock);
    }
    
    soLock[myNumber]->Acquire();
    soLineLock.Release();
    
    waitingForTicket_SO_C[myNumber]->Wait(soLock[myNumber]);
    waitingForTicket_SO_C[myNumber]->Signal(soLock[myNumber]);

    // Determine if the passenger passes or fails
    int randomNum = rand() % 100;
    if(randomNum < probabilityPassingSO) {
      //passenger passed
      so_passOrFail[myNumber] = true;
      printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n", myNumber,soPassenger[myNumber]);
    } else {
      //passenger failed
      so_passOrFail[myNumber] = false;
      printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n", myNumber,soPassenger[myNumber]);
    }
    /*
    siLineLock.Acquire();
    int passengerLine = findShortestLine(alLineLengths,7);

    siLineLock.Release();
    */
    // siLineLock.Acquire();
    // Search for an available SI
    /*
    bool foundAvailableSO = false;
    while( !foundAvailableSO )
      {
	siLineLock.Acquire();
	printf("stuck inside while loop\n");
        for(int i = 0; i < numberOfSO; i++)
          {
            if( !(si_busy[i]) ) {
              passengerGoToSI[ soPassenger[myNumber] ] = i;
              si_busy[i] = true;
	      foundAvailableSO = true;
              break;
            }
          }
	siLineLock.Release();
	
      }
    */
    // siLineLock.Release();

    // Clear passenger and direct to Security Inspector
    socount++;
    printf("Screening officer %d directs passenger %d to security inspector %d\n", myNumber, soPassenger[myNumber], passengerGoToSI[ soPassenger[myNumber] ]);
   
    soLock[myNumber]->Release();
  }
}

// Objects for Airport Liaison
Condition *waitingForAL_C[numberOfAL];
Condition *waitingForTicket_AL_C[numberOfAL];
Lock alLineLock("al_LL");
Lock *alLock[numberOfAL];
int alLineLengths[numberOfAL];
bool al_busy[numberOfAL];
int alPassenger[numberOfAL];
//int alPassengerCount = 0;

void AirportLiaison(int myNumber) {
  while(true) {
    // Acquire the Line Lock
    // No one can acquire the line lock (not Passengers)
    // When the Airport Liaison has the LineLock, Passengers cannot search for the shortest
    // line
    alLineLock.Acquire();

    // If there are passengers in the line, then
    // the Airport Liaison must tell the Passenger to step up to the counter
    // He does this by Signaling the Condition Variable which puts the first Passenger
    // on to the Ready Queue
    if(alLineLengths[myNumber]>0) {
      // printf("%s telling Passenger to step up to counter\n",currentThread->getName());
      // The first passenger waiting for the LineLock gets put on to the Ready Queue
      waitingForAL_C[myNumber]->Signal(&alLineLock);
    } else {
      // Airport Liaison is not busy if there is no one in line
      al_busy[myNumber] = false;

    }
    
    // Acquire the lock to the Airport Liaison
    // We will use this lock to control the interactions between the Passenger
    // and the Airport Liaison
    alLock[myNumber]->Acquire();
    
    // After acquiring that lock, we release the Line Lock so who ever is waiting for the 
    // Line Lock can then search for the shortest line and then get into the appropriate line 
    alLineLock.Release();
    
    if(current_test == 1) {
      currentThread->Finish();
    }
    // The Airport Liaison must now wait for the Passenger to go up to their counter 
    // and give them their ticket 
    // Sleeping the Airport Liaison frees up the alLock, wakes up one Passenger and puts them on the 
    // Ready Queue 
    waitingForTicket_AL_C[myNumber]->Wait(alLock[myNumber]);
    
    // Count the passenger's baggage
    /*
    al_baggage_buffer[alPassenger[myNumber]] = passenger_baggage_buffer[alPassenger[myNumber]];
    printf("%s takes note that Passenger %d has %d pieces of luggage\n",currentThread->getName(),alPassenger[myNumber],passenger_baggage_buffer[alPassenger[myNumber]]);
    */
    int flight_number = pass_ticket_buffer[alPassenger[myNumber]].flight_number;   
    al_baggage_buffer[flight_number] += baggage_buffer[alPassenger[myNumber]].numberOfBags;
    // printf("Flight %d has %d bags\n", flight_number,al_baggage_buffer[flight_number]);
    // The Airport Liaison signals a Passenger, who is asleep waiting for the Airport
    // Liaison to tell them where to go
    waitingForTicket_AL_C[myNumber]->Signal(alLock[myNumber]);
    pass_ticket_buffer[alPassenger[myNumber]].checkin_counter = pass_ticket_buffer[alPassenger[myNumber]].flight_number;  
    //pass_ticket_buffer[alPassengerpmyNumber]].checkin_counter = 
    printf("Airport Liason %d directed passenger %d of airline %d\n", myNumber, alPassenger[myNumber], pass_ticket_buffer[alPassenger[myNumber]].flight_number);
    alPassengerCount++;
    alLock[myNumber]->Release();
  }
}


// Objects for Check In Staff
Condition *waitingForCIS_C[numberOfCIS];
Condition *waitingForTicket_CIS_C[numberOfCIS];
Condition *onBreakCIS_C[numberOfCIS];
Condition *execLineCV[numberOfAirlines];
Condition *waitingForExec_CIS_C[numberOfCIS];

Lock *cisLineLock[numberOfAirlines];
Lock *cisLock[numberOfCIS];
Lock *execLineLock[numberOfAirlines];
Lock *execCISLock[numberOfCIS];

int cisLineLengths[numberOfCIS];
bool cis_busy[numberOfCIS];
int execLineLengths[numberOfAirlines];
bool waitingForExec[numberOfCIS];

// Use this to keep track of passenger
int cisPassenger[numberOfCIS];

int seatNumber[numberOfAirlines];

//int cisPassengerCount = 0;
int cisBaggageWeight[numberOfAirlines]; // keep track of the weight for each airline

void CheckInStaff(int myNumber) {
  if((current_test == 1)||(current_test==2)||(current_test == 3)) {
    currentThread->Finish();
  } 
  while(true) {
    int myAirline;
    if(myNumber > 9) {
      myAirline = 2;
    } else if (myNumber > 4) {
      myAirline = 1;
    } else {
      myAirline = 0;
    }
    
    // Acquire the line lock
    // cisLineLock[myAirline]->Acquire();

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

    cisLineLock[myAirline]->Acquire();
    if(cisLineLengths[myNumber]==0 && execLineLengths[myAirline]==0) {
      // go on break
      onBreakCIS_C[myNumber]->Wait(cisLineLock[myAirline]);
    }
    
    // If executive line > 0
    // Help the executives
    
    // execLineLock[myAirline]->Acquire();
    if(execLineLengths[myAirline] > 0) {
      execLineLock[myAirline]->Acquire();
      waitingForExec[myNumber] = true;
      // Tell an executive that I am ready 
      execLineCV[myAirline]->Signal(execLineLock[myAirline]);
      // Now waiting for the executive to signal 
      // execLineCV[myAirline]->Wait(execLineLock[myAirline]);
    
      execCISLock[myNumber]->Acquire();
      execLineLock[myAirline]->Release();

      waitingForExec_CIS_C[myNumber]->Wait(execCISLock[myNumber]);
      waitingForExec_CIS_C[myNumber]->Signal(execCISLock[myNumber]);
 
      cisFlightCount[myAirline]++;

      int flight_number = pass_ticket_buffer[cisPassenger[myNumber]].flight_number;   
      printf("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n",myNumber,myAirline,cisLineLengths[myNumber]);
      printf("Airline check-in staff %d of airline %d informs executive passenger %d to board at gate %d\n",myNumber,flight_number,cisPassenger[myNumber], flight_number);
      
      // Add these bags to the total count fort a given airline, specified by Flight Number
      cis_baggage_buffer[flight_number] += baggage_buffer[cisPassenger[myNumber]].weight;
      
      // Now add these bags to the conveyor belt
      conveyorBelt[cisPassenger[myNumber]].airline_code = flight_number;
      conveyorBelt[cisPassenger[myNumber]].number_of_bags = baggage_buffer[cisPassenger[myNumber]].numberOfBags;
      conveyorBelt[cisPassenger[myNumber]].weight = baggage_buffer[cisPassenger[myNumber]].weight;
      printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n", myNumber,myAirline);
      
      // CIS not waiting for executive passenger anymore
      waitingForExec[myNumber]=false;
      execCISLock[myNumber]->Release();
    }
    // execLineLock[myAirline]->Release();
    // execCISLock[myNumber]->Release();

    // cisLineLock[myAirline]->Acquire();

    if(cisLineLengths[myNumber] > 0) {
      //printf("line %d has more than one passenger\n", myNumber);
      //cisLineLock[myAirline]->Acquire();
      waitingForCIS_C[myNumber]->Signal(cisLineLock[myAirline]);
      //printf("%s telling Passenger %d to come to counter\n", currentThread->getName(), cis_current_passenger_serving[myNumber]);
      cisLock[myNumber]->Acquire();
      
      cisLineLock[myAirline]->Release();
      

      waitingForTicket_CIS_C[myNumber]->Wait(cisLock[myNumber]);
      waitingForTicket_CIS_C[myNumber]->Signal(cisLock[myNumber]);

      int flight_number = pass_ticket_buffer[cisPassenger[myNumber]].flight_number;   

      printf("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",myNumber,myAirline,execLineLengths[myNumber]);

      // Give the Passenger a seat number
      boarding_pass_buffer[cisPassenger[myNumber]].seat_number = seatNumber[flight_number]++;
      printf("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",myNumber,myAirline,cisPassenger[myNumber],flight_number);
      // Add these bags to the total count fort a given airline, specified by Flight Number
      cis_baggage_buffer[flight_number] += baggage_buffer[cisPassenger[myNumber]].weight;
      
      // Now add these bags to the conveyor belt
      conveyorBelt[cisPassenger[myNumber]].airline_code = flight_number;
      conveyorBelt[cisPassenger[myNumber]].number_of_bags = baggage_buffer[cisPassenger[myNumber]].numberOfBags;

      printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n",myNumber,myAirline);

      //printf("%s giving Passenger %d ticket number and directing them to gate\n", currentThread->getName(), cis_current_passenger_serving[myNumber]);
      cisFlightCount[myAirline]++;
      cisPassengerCount++;
    }
      cisLock[myNumber]->Release();
  
  }
}

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

  int myFlightNumber;
  bool amExecutive;
  amExecutive = pass_ticket_buffer[myNumber].executive;
  //if(amExecutive)
  //printf("Passenger %d is an executive passenger\n",myNumber);

  // Set the Passenger's Line number
  //printf("%s: Searching for the shortest line\n", currentThread->getName());
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
  if(current_test == 1) {
    //printf("Passenger %d chose Liaison %d with a line of length %d\n",myNumber,myLineNumber,alLineLengths[myLineNumber]);
    currentThread->Finish();
  }
  alLock[myLineNumber]->Acquire();

  //printf("Passenger %d chose Liaison %d with a line of length %d\n",myNumber,myLineNumber,alLineLengths[myLineNumber]);
  alLineLengths[myLineNumber]--;

  // Passenger is told to go to counter, and hands their ticket to Liaison
  alPassenger[myLineNumber] = myNumber;
  waitingForTicket_AL_C[myLineNumber]->Signal(alLock[myLineNumber]);
  waitingForTicket_AL_C[myLineNumber]->Wait(alLock[myLineNumber]);

  printf("Passenger %d of Airline %d is directed to the check-in counter\n",myNumber,pass_ticket_buffer[myNumber].flight_number);

  alLock[myLineNumber]->Release();
  
  if(current_test == 2) {
    // printf("test 2 passenger thread finishing");
    currentThread->Finish();
  }

  // --------------------------------------------------------
  // 2. Passenger goes to see Airport check in staff
  //
  //
  // --------------------------------------------------------

  // Acquire the Lock to the line
  // Only use one lock for all 5 lines, because only one Passenger at 
  // a time can be looking for the shortest line 
  int checkin_counter_number = pass_ticket_buffer[myNumber].checkin_counter;
  

  int start, stop;
  start = (pass_ticket_buffer[myNumber].checkin_counter)*(numberOfCIS/3);
  stop  = start + (numberOfCIS/3) - 1;
  // Figure out which 
  
  if(amExecutive) {
    execLineLock[checkin_counter_number]->Acquire();
    printf("Passenger %d of Airline %d chose is waiting in the executive class line\n",myNumber,pass_ticket_buffer[myNumber].flight_number);
    execLineLengths[checkin_counter_number]++;
    execLineCV[checkin_counter_number]->Wait(execLineLock[checkin_counter_number]);
    if(current_test == 3) {
      currentThread->Finish();
    }
    // Find the line of the Check in staff
    for(int i = start; i <= stop; i++) {
      if(waitingForExec[i] == true) {
        myLineNumber = i;
        break;
      }
    }
    
    // Tell CIS that passenger is ready
    // execLineCV[checkin_counter_number]->Signal(execLineLock[checkin_counter_number]);
    //printf("Passenger %d of Airline %d is waiting in the executive class line\n", currentThread->getName(), pass_ticket_buffer[myNumber].flight_number);
    // Waiting for CIS to give them boarding pass
    // execLineCV[checkin_counter_number]->Wait(execLineLock[checkin_counter_number]);
    execLineLengths[checkin_counter_number]--;
  
    execCISLock[myLineNumber]->Acquire();
    cisPassenger[myLineNumber] = myNumber;
    execLineLock[checkin_counter_number]->Release();
    
    waitingForExec_CIS_C[myLineNumber]->Signal(execCISLock[myLineNumber]);
    waitingForExec_CIS_C[myLineNumber]->Wait(execCISLock[myLineNumber]);
    execCISLock[myLineNumber]->Release();
  
  } else {

    cisLineLock[checkin_counter_number]->Acquire();

    // Set the Passenger's line number
    myLineNumber = findCISShortestLine(cisLineLengths,start,stop);
 
    cisLineLengths[myLineNumber]++;
    onBreakCIS_C[myLineNumber]->Signal(cisLineLock[checkin_counter_number]);
    cis_current_passenger_serving[myLineNumber] = myNumber;
    printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", myNumber, pass_ticket_buffer[myNumber].flight_number, myLineNumber, cisLineLengths[myLineNumber]);

    waitingForCIS_C[myLineNumber]->Wait(cisLineLock[checkin_counter_number]);
    if(current_test == 3) {
      currentThread->Finish();
    }
    // cisLineLengths[myLineNumber]--;
    
    //printf("%s going to see Airline Check In Staff %d\n",currentThread->getName(), myLineNumber); 
    cisLock[myLineNumber]->Acquire();

    cisPassenger[myLineNumber]=myNumber;

    cisLineLengths[myLineNumber]--;
    cisLineLock[checkin_counter_number]->Release();
    //cisLock[myLineNumber]->Acquire();
    
    
    // The Passenger now has the line number, so they should go to sleep and
    // release the line lock, letting another Passenger search for a line
    //printf("%s giving airline ticket to Airline Check In Staff %d\n", currentThread->getName(), myLineNumber);
    waitingForTicket_CIS_C[myLineNumber]->Signal(cisLock[myLineNumber]);
    waitingForTicket_CIS_C[myLineNumber]->Wait(cisLock[myLineNumber]);
    cisLock[myLineNumber]->Release();

    printf("Passenger %d of Airline %d was informed to board at gate %d\n",myNumber,pass_ticket_buffer[myNumber].flight_number,pass_ticket_buffer[myNumber].flight_number);
  }
  if((current_test == 4)||(current_test == 6)) {
    currentThread->Finish();
  }
  // --------------------------------------------------------
  // 3. Passenger goes to see Airport Security Officer
  //
  //
  // --------------------------------------------------------
  
  soLineLock.Acquire();
  
  myLineNumber = findShortestLine(soLineLengths, 7);

  soLineLengths[myLineNumber]++;
  printf("Passenger %d gives the hand-luggage to screening officer %d\n", myNumber, myLineNumber);
  waitingForSO_C[myLineNumber]->Signal(&soLineLock);
  waitingForSO_C[myLineNumber]->Wait(&soLineLock);
  
  soLineLengths[myLineNumber]--;
  soLineLock.Release();

  soLock[myLineNumber]->Acquire();

  soPassenger[myLineNumber] = myNumber;

  // The Passenger now has the line number, so they should go to sleep and
  // release the line lock, letting another Passenger search for a line
  waitingForTicket_SO_C[myLineNumber]->Signal(soLock[myLineNumber]);
  //printf("Passenger %d gives the hand-luggage to screening officer %d\n", myNumber, myLineNumber);
  waitingForTicket_SO_C[myLineNumber]->Wait(soLock[myLineNumber]);

  soLock[myLineNumber]->Release();

  if(current_test == 7)
    currentThread->Finish();
  // --------------------------------------------------------
  // 4. Passenger goes to see Airport Security Inspector
  //
  //
  // --------------------------------------------------------
  
  siLineLock.Acquire();
  
  myLineNumber = findShortestLine(siLineLengths, 7);

  // myLineNumber = passengerGoToSI[myNumber];
  printf("Passenger %d moves to security inspector %d\n",myNumber,myLineNumber);

  siLineLengths[myLineNumber]++;
  //printf("%s: chose SInspect %d with length %d\n", currentThread->getName(), myLineNumber, siLineLengths[myLineNumber]);
  
  waitingForSI_C[myLineNumber]->Signal(&siLineLock);
  waitingForSI_C[myLineNumber]->Wait(&siLineLock);
  
  siLineLengths[myLineNumber]--;
  siLock[myLineNumber]->Acquire();
  siLineLock.Release();
  
  //siLock[myLineNumber]->Acquire();
  siPassenger[myLineNumber] = myNumber;

  // The Passenger now has the line number, so they should go to sleep and
  // release the line lock, letting another Passenger search for a line
  //printf("%s giving airline ticket to Security Inspector %d\n", currentThread->getName(), myLineNumber);
  waitingForTicket_SI_C[myLineNumber]->Signal(siLock[myLineNumber]);
  waitingForTicket_SI_C[myLineNumber]->Wait(siLock[myLineNumber]);

  siLock[myLineNumber]->Release();
  
 
  //if(passengersFailedSI[myNumber]) {
  /*
  if(false) {  
  //going to further questioning
    printf("Passenger %d goes for further questioning\n",myNumber);
    for(int i = 0; i < 10; i++)
      currentThread->Yield();
    
    // Passenger now comes back 
    siLineLock.Acquire();

    // This works like the executive line
    siLineLengths[myLineNumber]++;
    //siBackFromQuestioningLineLengths[myLineNumber]++;
    //waitingForSIAfterQuestioning_C[myLineNumber]->Signal(&siLineLock);
    //waitingForSIAfterQuestioning_C[myLineNumber]->Wait(&siLineLock);

    // The passenger will return to their original airport inspector
    if(current_test == 8) {
      printf("Passenger %d comes back to security inspector %d after further examination\n",myNumber,myLineNumber);
    }

    waitingForSI_C[myLineNumber]->Signal(&siLineLock);
    waitingForSI_C[myLineNumber]->Wait(&siLineLock);


    //siBackFromQuestioningLineLengths[myLineNumber]--;
    siLineLengths[myLineNumber]--;

    siLineLock.Release();
    
    siLock[myLineNumber]->Acquire();
    siPassenger[myLineNumber] = myNumber;
  }
  */
  //printf("-----Number of Passengers chosen inspector: %d\n",pass_si_count);
  pass_si_count++;

  // --------------------------------------------------------
  // 5. Passenger goes to boarding lounge
  //
  //
  // --------------------------------------------------------
  if(current_test>0) {
    currentThread->Finish();
  }
  
  myFlightNumber = boarding_pass_buffer[myNumber].flight_number;
  
  //flightCount[myFlightNumber]++;
  airlineLock[myFlightNumber]->Acquire();
  printf("passenger %d is at the boarding lounge\n", myNumber);
  flightCount[myFlightNumber]++;
  waitingForCallAM_C[myFlightNumber]->Wait(airlineLock[myFlightNumber]);
  airlineLock[myFlightNumber]->Release();
  printf("Passenger %d of Airline %d boarded airline %d\n", myNumber,myFlightNumber,myFlightNumber);
  
  // FIN
  
}

void AirportSimulation() {

  Thread *t; // Create a thread pointer variable
  char *name;
  int i; 

  /*
   * Needs Airlines
   * Bags and weights
   *
   */

  //printf("Starting Airport Simulation\n");


  // -------------------------------------------------
  // Initialize Condition Variables
  
  // waitingForAL condition variable
  for(i = 0; i < numberOfAL; i++) {
    name = new char [20];
    sprintf(name, "WFAL_C%d",i);
    waitingForAL_C[i] = new Condition(name);
    al_busy[i] = true;
  }

  // waitingForCIS condition variable
  for(i = 0; i < numberOfCIS; i++) {
    name = new char [20];
    sprintf(name,"WFCIS_C%d",i);
    waitingForCIS_C[i] = new Condition(name);
    cis_busy[i] = true;
  }

  // waitingForSO condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char [20];
    sprintf(name,"WFSO_C%d",i);
    waitingForSO_C[i] = new Condition(name);
    so_busy[i] = true;
  }

  // waitingForSI condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char [20];
    sprintf(name,"WFSI_C%d",i);
    waitingForSI_C[i] = new Condition(name);
    si_busy[i] = true;
  }

  // waitingForTicket_AL condition variable
  for(i = 0; i < numberOfAL; i++) {
    name = new char [20];
    sprintf(name, "WFTICKET_AL_C%d",i);
    waitingForTicket_AL_C[i] = new Condition(name);
  }
  
  // waitingForTicket_CIS_C condition variable
  for(i = 0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name,"CISTICKET_C%d",i);
    waitingForTicket_CIS_C[i] = new Condition(name);
  }

  // waitingForTicket_SO_C condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"SOTICKET_C%d",i);
    waitingForTicket_SO_C[i] = new Condition(name);
  }

  // waitingForTicket_SI_C condition variable
  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"SITICKET_C%d",i);
    waitingForTicket_SI_C[i] = new Condition(name);
  }

  // onBreakCIS_C condition variable
  for(i = 0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name, "CISBREAK_C%d",i);
    onBreakCIS_C[i] = new Condition(name);
  }
   
  // waitingForCallAM_C condition variable
  for(i = 0; i < numberOfAirlines; i++) {
    name = new char[20];
    sprintf(name, "AM_C%d",i);
    waitingForCallAM_C[i] = new Condition(name);
  }
  //--------------------------------------------------

  // -------------------------------------------------
  // Initialize Airport Liaison Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfAL; i++) {
    name = new char[20];
    sprintf(name,"alLock%d",i);
    alLock[i] = new Lock(name);
  }
  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize Check in Staff Line Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfAirlines; i++) {
    name = new char[20];
    sprintf(name,"cisLineLock%d",i);
    cisLineLock[i] = new Lock(name);
  }

  for(i=0; i < numberOfAirlines; i++) {
    name = new char[20];
    sprintf(name,"execLineLock%d",i);
    execLineLock[i] = new Lock(name);
    name = new char [20];
    sprintf(name,"execLineCV%d",i);
    execLineCV[i]  = new Condition(name);
  }
  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize Airline check in staff Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfCIS; i++) {
    name = new char[20];
    sprintf(name,"cisLock%d",i);
    cisLock[i] = new Lock(name);
    name = new char[20];
    sprintf(name,"cisExecLock%d",i);
    execCISLock[i] = new Lock(name);
    name = new char[20];
    sprintf(name,"waitingForExec_CIS_C%d",i);
    waitingForExec_CIS_C[i] = new Condition(name);
  }
  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize Security Officer check in staff Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"soLock%d",i);
    soLock[i] = new Lock(name);
  }

  for(i = 0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name,"siLock%d",i);
    siLock[i] = new Lock(name);
  }
  // -------------------------------------------------
  // -------------------------------------------------
  // Initialize Security Officer check in staff Locks
  // printf("creating al locks\n");
  for(i = 0; i < numberOfAirlines; i++) {
    name = new char[20];
    sprintf(name,"airLineLock%d",i);
    airlineLock[i] = new Lock(name);
  }

  // -------------------------------------------------

  // -------------------------------------------------
  // Initialize the Line Lengths 

  // Line length for Airport Liaison
  for( i = 0; i < numberOfAL; i++) {
    alLineLengths[i] = 0;
  }

  // Line length for Airline check in staff
  for( i = 0; i < numberOfCIS; i++) {
    cisLineLengths[i] = 0;
    cis_current_passenger_serving[i] = 0;
  }

  // Line length for screening officers
  for( i = 0; i < numberOfSO; i++) {
    soLineLengths[i] = 0;
  }

  // Line length for security inspectors
  for( i = 0; i < numberOfSO; i++) {
    siLineLengths[i] = 0;
  }
  for(i=0; i < numberOfAirlines; i++) {
    execLineLengths[i] = 0;
  }
  // -------------------------------------------------


  for(i=0; i < numberOfAirlines; i++) {
    flightCount[i]=0;
    cisFlightCount[i]=0;
    siAirlineCount[i]=0;
  }

  for(i = 0; i < numberOfAirlines; i++) {
    alreadyCalled[i] = false; 
  }

  // Initialize the baggage buffer
  for(i = 0; i < numberOfAirlines; i++) {
    al_baggage_buffer[i]        = 0;
    cis_baggage_buffer[i]       = 0;
    cargoHandlerBaggageCount[i] = 0;
    seatNumber[i]               = 0;
  }

  // Create the 20 passenger for our airport simulation
  printf("Creating Passengers\n");
  for( i=0; i < numberOfPassengers; i++) {
    // Create a ticket for the passenger
    pass_ticket_buffer[i].passenger_number = i;
    pass_ticket_buffer[i].flight_number = (i%numberOfAirlines);
    if((i%8)==0) {
      pass_ticket_buffer[i].executive = true;
    } else {
      pass_ticket_buffer[i].executive = false;
    }
    pass_ticket_buffer[i].checkin_counter = -1;
    // TO DO 
    // Randomize weights
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
    baggage_buffer[i].passenger_number = i;
    baggage_buffer[i].airline_code = (i%numberOfAirlines);
    boarding_pass_buffer[i].passenger_number = i;
    boarding_pass_buffer[i].flight_number = (i%numberOfAirlines);
    boarding_pass_buffer[i].seat_number = -1;

    name = new char [20]; 
    sprintf(name,"Passenger%d",i);
    //printf("Creating %s\n",name);
    t = new Thread(name); // Give the Passenger a name
    t->Fork((VoidFunctionPtr)Passenger,i);
  }

  // Create all the Airport Staff First

  // Create the Airport Liaison
  for(i = 0; i < numberOfAL; i++) {
    name = new char[20];
    sprintf(name, "AL%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)AirportLiaison,i);
  }

  // Create the Airline Check In Staff
  for(i=0; i < numberOfCIS; i++) {
    waitingForExec[i] = false;
    name = new char[20];
    sprintf(name, "Airline check-in-staff%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)CheckInStaff,i);
  }
  
  // Create the Security Officer Staff
  for(i=0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name, "SecurityOfficer%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)SecurityOfficer,i);
  }
  
  // Create the Airline Check In Staff
  
  for(i=0; i < numberOfSO; i++) {
    name = new char[20];
    sprintf(name, "SecurityInspector%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)SecurityInspector,i);
  }

  for(i=0; i < numberOfCH; i++) {
    name = new char[20];
    sprintf(name, "CargoHandler%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)CargoHandler,i);
  }
  
  name = new char[20];
  name = "AirportManager";
  t = new Thread(name);
  t->Fork((VoidFunctionPtr)AirportManager,1);
  
  if(current_test==0) {
    printf("Number of airport liasons = %d\n",numberOfAL);
    printf("Number of airlines = %d\n",numberOfAirlines);
    printf("Number of check-in staff = %d\n",numberOfCIS);
    printf("Number of cargo handlers = %d\n",numberOfCH);
    printf("Number of screening officers = %d\n",numberOfSO);
    printf("Total number of passengers = %d\n",numberOfPassengers);
    
    for(int i=0; i < numberOfAirlines; i++) {
      int numPassengersOnAirline = 0;
      for(int j=0; j < numberOfPassengers; j++) {
	if(pass_ticket_buffer[j].flight_number == i)
	  numPassengersOnAirline++;
      }
      printf("Number of passengers for airline %d = %d\n",0,numPassengersOnAirline);
    }
    
    for(int i=0; i < numberOfPassengers; i++) {
      printf("Passenger %d belongs to airline %d\n",i,pass_ticket_buffer[i].flight_number);
      printf("Passenger %d: Number of bags = %d\n",i,baggage_buffer[i].numberOfBags);
      printf("Passenger %d: Weight of bags = ",i);
      for(int j = 0; j < baggage_buffer[i].numberOfBags; j++) {
	if( j!=0 && j!= (baggage_buffer[i].numberOfBags))
        printf(",");
	printf("%d",baggage_buffer[i].weight);
      }
      printf("\n");
    }
    
    for(int i=0; i < numberOfCIS; i++) {
      printf("Airline check-in staff %d belongs to airline %d\n",i,i);
    }
  }
}

void Test1() {
  printf("Starting Test One\n");
  current_test = 1;
  AirportSimulation();
}

void Test2() {
  printf("Starting Test Two\n");
  current_test = 2;
  AirportSimulation();
}

void Test3() {
  printf("Starting Test Three\n");
  current_test = 3;
  AirportSimulation();
}

void Test4() {
  printf("Starting Test Four\n");
  current_test = 4;
  AirportSimulation();
}

void Test5() {
  printf("Starting Test Five\n");
  current_test = 5;
  AirportSimulation();
}

void Test6() {
  char* name;
  onBreak_CH = false;
  printf("Starting Test Six\n");
  current_test = 6;
  int i;
  for(i=0; i <numberOfPassengers; i++) {
    conveyorBelt[i].number_of_bags = (i%2)+2;
    conveyorBelt[i].airline_code = (i%3);
    conveyorBelt[i].weight = 60;
  }
  Thread *t;
  for(i=0; i < numberOfCH; i++) {
    name = new char[20];
    sprintf(name, "CargoHandler%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)CargoHandler,i);
  }
}

void Test7() {
  printf("Starting Test Seven\n");
  current_test = 7;
  AirportSimulation();
}

void Test8() {
  printf("Starting Test Eight\n");
  current_test = 8;
  AirportSimulation();
}

void Test9() {
  printf("Starting Test Nine\n");
  // current_test = 9;
  AirportSimulation();
}

void Test10() {
  printf("Starting Test Ten\n");
  current_test = 10;
  AirportSimulation();
}


//----------------------------------------------------------------------
// ThreadTest
//      Set up a ping-pong between two threads, by forking a thread 
//      to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
  DEBUG('t', "Entering SimpleTest");
  /*
    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
was she p    SimpleThread(0);
  */
  // TestSuite();
}
