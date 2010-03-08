/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "functions.h"
#include "syscall.h"
int a[3];
int b, c;
/*
void hello() {
  Write("hello\n",6,1);
  Exit(0);
}
void world() {
  Write("world\n",6,1);
  Exit(0);
}
int main()
{ 
  int z;
  Write("Testing halt\n",13,1);

  Write("Testing fork\n",13,1);
  for(z = 0; z < 1; z++) {
    Fork(hello);
    Fork(world);
  }
  Exit(0);
}
*/


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

int consoleLock;

void AirportManager(int myNumber) {
	Write("Forking Airport Manager\n",24,ConsoleOutput);
	Exit(0);
}

void CargoHandler(int myNumber) {
	Write("Forking Cargo Handler\n",22,ConsoleOutput);
	Exit(0);
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
	Write("Forking Security Inspector\n",27,ConsoleOutput);
	Exit(0);
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
	Write("Forking Security Officer\n",25,ConsoleOutput);
	Exit(0);
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
	Write("Forking Check In Staff\n",23,ConsoleOutput);
	Exit(0);
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
	Write("Forking Airport Liaison\n",24,ConsoleOutput);
	Exit(0);
}


void Passenger(int myNumber) {
	Write("Forking Passenger\n",18,ConsoleOutput);
	Exit(0);
}


int main () {

  int i, b;
  int randNumBags, randNumWeight;
  /* Thread *t; Create a thread pointer variable */
  char *name;
  consoleLock = CreateLock(1);

  /*
   * Needs Airlines
   * Bags and weights
   *
   */

  /* printf("Starting Airport Simulation\n"); */


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
    Fork(Passenger);
  }

  /* Create all the Airport Staff First */

  /* Create the Airport Liaison */
  
  for(i = 0; i < numberOfAL; i++) {
    /*
    name = new char[20];
    sprintf(name, "AL%d",i);
    t = new Thread(name);
    */
    Fork(AirportLiaison);
  }

  /* Create the Airline Check In Staff */

  for(i=0; i < numberOfCIS; i++) {
    waitingForExec[i] = 0;
    /*
    name = new char[20];
    sprintf(name, "Airline check-in-staff%d",i);
    t = new Thread(name);
    */
    Fork(CheckInStaff);
  }
  
  /* Create the Security Officer Staff */

  for(i=0; i < numberOfSO; i++) {
    /*
    name = new char[20];
    sprintf(name, "SecurityOfficer%d",i);
    t = new Thread(name);
    */
    Fork(SecurityOfficer);
  }
  
  /* Create the Airline Check In Staff */

  for(i=0; i < numberOfSO; i++) {
    /*
    name = new char[20];
    sprintf(name, "SecurityInspector%d",i);
    t = new Thread(name);
    */
    Fork(SecurityInspector);
  }

  for(i=0; i < numberOfCH; i++) {
    /*
    name = new char[20];
    sprintf(name, "CargoHandler%d",i);
    t = new Thread(name);
    */
    Fork(CargoHandler);
  }
  /*
    name = new char[20];
    name = "AirportManager";
    t = new Thread(name);
  */

  Fork(AirportManager);
  
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
    Fork(CargoHandler);
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
