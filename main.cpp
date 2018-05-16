//Damien Sudol
//CPSC5042 HW3
//Version 1.0

#include <string>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

//thread prototypes
void * Dentist(void * unused);
void * Customer(void * i);

//declare semaphores
sem_t dentistReady;
sem_t seatCountWriteAccess;
sem_t patientReady;



//declare/initialize locks for output and seat increments
pthread_mutex_t atomicLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t patientCountLock = PTHREAD_MUTEX_INITIALIZER;

struct DC
{
int numberOfFreeWRSeats = 0;
int patientCount = 0;
};

int main(int argc, char *argv[]){

struct DC * dc;
dc = new struct DC;

//initialize semaphores to their starting values
sem_init(&dentistReady, 0, 0);
sem_init(&seatCountWriteAccess, 0, 1);
sem_init(&patientReady, 0, 0);

//cast command line input to integers and assign to variables
dc->numberOfFreeWRSeats = stoi(argv[1]);
int totalPatients = stoi(argv[2]);

//create thread ids for all necessary threads
pthread_t customerID[totalPatients];
pthread_t dentistThreadID;

//create 1 dentist thread
pthread_create(&dentistThreadID, NULL, Dentist, (void*) dc);

//use for loop to create an equal number of Customer threads
//to second integer argument from user
for(int i = 0; i < totalPatients; i++)
	pthread_create(&customerID[i], NULL, Customer, (void*) dc);

//join used as barrier to prevent process from exiting while threads running
pthread_join(dentistThreadID, NULL);

return 0;
}


void * Dentist(void * dc)
{
struct DC * dcStruct = (struct DC *) dc;

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
		dcStruct->numberOfFreeWRSeats +=1;
		cout << "Incremented free seats to " << dcStruct->numberOfFreeWRSeats << endl;	
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

return (void*) dcStruct;

}

void * Customer(void * dc)
{
struct DC * dcStruct = (struct DC *) dc;
int activeCustomer;
pthread_mutex_lock(&patientCountLock);
	dcStruct->patientCount++;
	activeCustomer = dcStruct->patientCount;
pthread_mutex_unlock(&patientCountLock);


while(true)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Customer " << activeCustomer << " trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);

	sem_wait(&seatCountWriteAccess);


	if(dcStruct->numberOfFreeWRSeats > 0)
	{

	pthread_mutex_lock(&atomicLock);
		dcStruct->numberOfFreeWRSeats -= 1;
		cout << "Customer "<<activeCustomer<<" seated; Remaining chairs= "<<dcStruct->numberOfFreeWRSeats<< "."<< endl;
	pthread_mutex_unlock(&atomicLock);
		
	pthread_mutex_lock(&atomicLock);
		cout << "Customer " << activeCustomer << " notifying dentist patientReady." << endl;
	pthread_mutex_unlock(&atomicLock);
			
	sem_post(&patientReady);

	pthread_mutex_lock(&atomicLock);
	 	cout << "Customer " << activeCustomer << " releasing seatCountWriteAccess..." << endl;
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
		cout << "Customer " << activeCustomer << " leaving without consulting; no chairs available..." << endl;
	pthread_mutex_unlock(&atomicLock);

	sem_post(&seatCountWriteAccess);

	}
	}

return (void*) dcStruct;
}
