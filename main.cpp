#include <string>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

void * Dentist(void * unused);
void * Customer(void * unused);

sem_t dentistReady;
sem_t seatCountWriteAccess;
sem_t patientReady;
pthread_cond_t atomic;
pthread_mutex_t atomicLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t patientCountLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t seatCountLock = PTHREAD_MUTEX_INITIALIZER;

struct dentistry
{
pthread_t id[15];
int numberOfFreeWRSeats;
int patientCount = 0;
int totalPatients;
};

int main(){

struct dentistry * dStruct;
dStruct = new struct dentistry;

sem_init(&dentistReady, 0, 0);
sem_init(&seatCountWriteAccess, 0, 1);
sem_init(&patientReady, 0, 0);

string strSeatCount;
string strTotalPatients;



cout << "Enter Number of Seats: " << endl;
cin >> strSeatCount;
cout << "Enter Number of Patients: " << endl;
cin >> strTotalPatients;

int numberOfFreeSeatsReset = dStruct->numberOfFreeWRSeats = stoi(strSeatCount);
int totalPatientsReset = dStruct->totalPatients = stoi(strTotalPatients);

pthread_create(&dStruct->id[0], NULL, Dentist, (void*) dStruct);
pthread_create(&dStruct->id[1], NULL, Customer, (void*) dStruct);

pthread_join(dStruct->id[0], NULL);
pthread_join(dStruct->id[1], NULL);

dStruct->patientCount = 0;
dStruct->numberOfFreeWRSeats = numberOfFreeSeatsReset;

pthread_create(&dStruct->id[2], NULL, Dentist, (void*) dStruct);
pthread_create(&dStruct->id[3], NULL, Customer, (void*) dStruct);
pthread_create(&dStruct->id[4], NULL, Customer, (void*) dStruct);

for(int i = 2; i < 5; i++)
	pthread_join(dStruct->id[i], NULL);

dStruct->patientCount = 0;
dStruct->numberOfFreeWRSeats = numberOfFreeSeatsReset;

pthread_create(&dStruct->id[5], NULL, Dentist, (void*) dStruct);
for(int i = 6; i < 9; i++)
	pthread_create(&dStruct->id[i], NULL, Customer, (void*) dStruct);

for(int i = 5; i < 9; i++)
	pthread_join(dStruct->id[i], NULL);

dStruct->patientCount = 0;
dStruct->numberOfFreeWRSeats = numberOfFreeSeatsReset;

pthread_create(&dStruct->id[10], NULL, Dentist, (void*) dStruct);
for(int i = 11; i < 16; i++)
	pthread_create(&dStruct->id[i], NULL, Customer, dStruct);

for(int i = 10; i < 16; i++)
	pthread_join(dStruct->id[i], NULL);

return 0;
}

void * Dentist(void * dStruct)
{

struct dentistry * dS = (struct dentistry *) dStruct;

while(dS->patientCount <= dS->totalPatients)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Dentist trying to aquire patient..." << endl;
	pthread_mutex_unlock(&atomicLock);

	sem_wait(&patientReady);

	pthread_mutex_lock(&atomicLock);
		cout << "Dentist trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	
	sem_wait(&seatCountWriteAccess);
		pthread_mutex_lock(&seatCountLock);
		dS->numberOfFreeWRSeats +=1;
		pthread_mutex_unlock(&seatCountLock);

		pthread_mutex_lock(&atomicLock);
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

void * Customer(void * dStruct)
{
struct dentistry * dS = (struct dentistry *) dStruct;
int activeCustomer;

pthread_mutex_lock(&patientCountLock);
	dS->patientCount++;
	activeCustomer = dS->patientCount;
pthread_mutex_unlock(&patientCountLock);

while(dS->patientCount <= dS->totalPatients)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Customer " << activeCustomer << " trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	sem_wait(&seatCountWriteAccess);
	
		if(dS->numberOfFreeWRSeats > 0)
		{
			pthread_mutex_lock(&seatCountLock);
			dS->numberOfFreeWRSeats -= 1;
			pthread_mutex_unlock(&seatCountLock);

			pthread_mutex_lock(&atomicLock);
				cout << "Customer " << activeCustomer << " seated; Remaining chairs = " <<
					dS->numberOfFreeWRSeats << "." << endl;
			pthread_mutex_unlock(&atomicLock);
			
			pthread_mutex_lock(&atomicLock);
				cout << "Customer " << activeCustomer << " notifying dentist patientReady." <<
					endl;
			pthread_mutex_unlock(&atomicLock);
			sem_post(&patientReady);

			pthread_mutex_lock(&atomicLock);
			 	cout << "Customer " << activeCustomer << " releasing seatCountWriteAccess..." <<
					endl;
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
			cout << "Customer " << activeCustomer << " leaving without consulting; " <<
				"no chairs available..." << endl;
		pthread_mutex_unlock(&atomicLock);
		sem_post(&seatCountWriteAccess);
	
		}
	pthread_mutex_lock(&patientCountLock);
		dS->patientCount++;
		activeCustomer = dS->patientCount;
	pthread_mutex_unlock(&patientCountLock);
	}



return (void*) dS;
}
