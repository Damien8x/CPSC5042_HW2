//Damien Sudol
//CPSC5042 HW3
//Version 1.0

#include <string>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

//declare thread prototypes
void * Dentist(void * dStruct);
void * Customer(void * dStruct);

//declare semaphores for Dentist/Customer threads
sem_t dentistReady;
sem_t seatCountWriteAccess;
sem_t patientReady;
//declare and initialize locks for printing statements to console(atomicLock)
//incrementing patientCount atomically(patientCountLock)
//and incrementing/decrementing available seats (atomicLock)
pthread_mutex_t atomicLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t patientCountLock = PTHREAD_MUTEX_INITIALIZER;

//Struct to store shared variables needed within threads and
//main driver
struct dentistry
{
pthread_t id[16];
int numberOfFreeWRSeats = 0;
int patientCount = 0;
int totalPatients = 0;
bool ready = true;
};

int main(){

//declare and initialize struct for threads and driver
struct dentistry * dStruct;
dStruct = new struct dentistry;

//initialize semaphores to their respective values, as per assignment details
sem_init(&dentistReady, 0, 0);
sem_init(&seatCountWriteAccess, 0, 1);
sem_init(&patientReady, 0, 0);

//declare strings to store user input for simulated seat count and number of 
//patients
string strSeatCount;
string strTotalPatients;

//prompts to user for input/store input in string variables
cout << "Enter Number of Seats: " << endl;
cin >> strSeatCount;
cout << "Enter Number of Patients: " << endl;
cin >> strTotalPatients;

//cast input to integer value. store in shared struct/reset variables
int numberOfFreeSeatsReset = dStruct->numberOfFreeWRSeats = stoi(strSeatCount);
int totalPatientsReset = dStruct->totalPatients = stoi(strTotalPatients);

//create 1:1 Doctor to Customer threads, passing in struct
pthread_create(&dStruct->id[0], NULL, Dentist, (void*) dStruct);
pthread_create(&dStruct->id[1], NULL, Customer, (void*) dStruct);

//join Customer thread. cancel Dentist thread post completion of Customer.
pthread_join(dStruct->id[1], NULL);
pthread_cancel(dStruct->id[0]);

//reset necessaary values for next test condition
dStruct->patientCount = 0;
dStruct->numberOfFreeWRSeats = numberOfFreeSeatsReset;
dStruct->ready = true;

//create 1:2 Doctor to Customer threads, passing in struct
pthread_create(&dStruct->id[2], NULL, Dentist, (void*) dStruct);
pthread_create(&dStruct->id[3], NULL, Customer, (void*) dStruct);
pthread_create(&dStruct->id[4], NULL, Customer, (void*) dStruct);

//for loop to join all Customer threads, followed by canceling 
//Doctor thread
for(int i = 3; i < 5; i++)
	pthread_join(dStruct->id[i], NULL);
pthread_cancel(dStruct->id[2]);

//resest necessary values for next test condition
dStruct->patientCount = 0;
dStruct->numberOfFreeWRSeats = numberOfFreeSeatsReset;
dStruct->ready = true;

//Create 1:3 Doctor to Customer threads, passing in struct
pthread_create(&dStruct->id[5], NULL, Dentist, (void*) dStruct);
for(int i = 6; i < 9; i++)
	pthread_create(&dStruct->id[i], NULL, Customer, (void*) dStruct);

//for loop to join all running Customer threads, followed by
//canceling running Doctor threqd
for(int i = 6; i < 9; i++)
	pthread_join(dStruct->id[i], NULL);
pthread_cancel(dStruct->id[5]);

//reset necessary values for next test condition
dStruct->patientCount = 0;
dStruct->numberOfFreeWRSeats = numberOfFreeSeatsReset;
dStruct->ready = true;

//Create 1:5 Doctor to Customer threads, passing in struct
pthread_create(&dStruct->id[10], NULL, Dentist, (void*) dStruct);
for(int i = 11; i < 16; i++)
	pthread_create(&dStruct->id[i], NULL, Customer, (void*) dStruct);

//for loop to join all currently running Customer threads,
//followed by canceling running Dotor thread
for(int i = 11; i < 16; i++)
	pthread_join(dStruct->id[i], NULL);
pthread_cancel(dStruct->id[10]);

return 0;
}

//Dentist thread, simulating conditions laid out in assignment
//takes in void paramteter
void * Dentist(void * dStruct)
{
//cast argument to dentistry struct and assign values to dS
struct dentistry * dS = (struct dentistry *) dStruct;

//infinite while loop, simulating logic detailed in assignment guidelines.
//All output share the SAME lock mutex to ensure output is pritned
//in their entirety.
while(true)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Dentist trying to aquire patient..." << endl;
	pthread_mutex_unlock(&atomicLock);

	sem_wait(&patientReady);

	pthread_mutex_lock(&atomicLock);
		cout << "Dentist trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	
	sem_wait(&seatCountWriteAccess);
		
		pthread_mutex_lock(&atomicLock);
		dS->numberOfFreeWRSeats +=1;
			cout << "Incremented free seats to " << dS->numberOfFreeWRSeats << endl;	
		pthread_mutex_unlock(&atomicLock);

	pthread_mutex_lock(&atomicLock);
		cout << "Dentist ready to consult..." << endl;
	pthread_mutex_unlock(&atomicLock);
	
	sem_post(&dentistReady);
	
	pthread_mutex_lock(&atomicLock);
		cout << "Dentist releasing seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	
	sem_post(&seatCountWriteAccess);

	pthread_mutex_lock(&atomicLock);
		cout << "Dentist consulting patient" << endl;
	pthread_mutex_unlock(&atomicLock);
	}

return (void*)dS;

}

//Customer thread
void * Customer(void * dStruct)
{
//cast void parameter to dentistry struck and assign to DS
struct dentistry * dS = (struct dentistry *) dStruct;
//declare activeCustomer to simulate the customer number entering the waiting room
int activeCustomer;

//increment patient count and assign value to active customer.
//patientCount,a shared variable is being modified, so a lock must be used.
pthread_mutex_lock(&patientCountLock);
	dS->patientCount++;
	activeCustomer = dS->patientCount;
pthread_mutex_unlock(&patientCountLock);

//while loop designed to exevute while patiencCount <= totalPatients
//while loop simulates logic laid out per assignment guidelines
//All output share the SAME lock mutex to ensure output is pritned
//in their entirety.
	pthread_mutex_lock(&patientCountLock);
	if(dS->patientCount > dS->totalPatients)
			dS->ready = false;
	pthread_mutex_unlock(&patientCountLock);

while(dS->ready)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Customer " << activeCustomer << " trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	sem_wait(&seatCountWriteAccess);
	
		if(dS->numberOfFreeWRSeats > 0)
		{
			
			pthread_mutex_lock(&atomicLock);
			dS->numberOfFreeWRSeats -= 1;
				cout << "Customer " << activeCustomer << " seated; Remaining chairs = "
				 << dS->numberOfFreeWRSeats << "." << endl;
			pthread_mutex_unlock(&atomicLock);
		
			
			pthread_mutex_lock(&atomicLock);
				cout << "Customer " << activeCustomer << " notifying dentist patientReady."
				 << endl;
			pthread_mutex_unlock(&atomicLock);
			sem_post(&patientReady);

			pthread_mutex_lock(&atomicLock);
			 	cout << "Customer " << activeCustomer << " releasing seatCountWriteAccess..."
				 << endl;
			pthread_mutex_unlock(&atomicLock);
			sem_post(&seatCountWriteAccess);

			pthread_mutex_lock(&atomicLock);
				cout << "Customer " << activeCustomer << " waiting for dentist..." << endl;
			pthread_mutex_unlock(&atomicLock);
			sem_wait(&dentistReady);
			pthread_mutex_lock(&atomicLock);
				cout << "Customer " << activeCustomer << " consulting dentist..." << endl;
			pthread_mutex_unlock(&atomicLock);
			
		}
		else
		{
		pthread_mutex_lock(&atomicLock);
			cout << "Customer " << activeCustomer
			 << " leaving without consulting; no chairs available..." << endl;
		pthread_mutex_unlock(&atomicLock);
		sem_post(&seatCountWriteAccess);
	
		}
	pthread_mutex_lock(&patientCountLock);
		dS->patientCount++;
		activeCustomer = dS->patientCount;
		//condition to ensure loop is not executed beyond intention.
		//sets shared variable "ready" to false if conditions are met.
		//this is reflected acccross all threads
		if(dS->patientCount > dS->totalPatients)
			dS->ready = false;
	pthread_mutex_unlock(&patientCountLock);
		
	}
return (void*) dS;
}
