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
pthread_t id[15];

int numberOfFreeWRSeats;
int patientCount = 0;
int totalPatients;
bool dentistHasPatients = true;

int main(){

sem_init(&dentistReady, 0, 0);
sem_init(&seatCountWriteAccess, 0, 1);
sem_init(&patientReady, 0, 0);

string strSeatCount;
string strTotalPatients;



cout << "Enter Number of Seats: " << endl;
cin >> strSeatCount;
cout << "Enter Number of Patients: " << endl;
cin >> strTotalPatients;

int numberOfFreeSeatsReset = numberOfFreeWRSeats = stoi(strSeatCount);
int totalPatientsReset = totalPatients = stoi(strTotalPatients);

pthread_create(&id[0], NULL, Dentist, NULL);
pthread_create(&id[1], NULL, Customer, NULL);

pthread_join(id[0], NULL);
pthread_join(id[1], NULL);

patientCount = 0;
numberOfFreeWRSeats = numberOfFreeSeatsReset;

pthread_create(&id[2], NULL, Dentist, NULL);
pthread_create(&id[3], NULL, Customer, NULL);
pthread_create(&id[4], NULL, Customer, NULL);

for(int i = 2; i < 5; i++)
	pthread_join(id[i], NULL);

patientCount = 0;
numberOfFreeWRSeats = numberOfFreeSeatsReset;

pthread_create(&id[5], NULL, Dentist, NULL);
for(int i = 6; i < 9; i++)
	pthread_create(&id[i], NULL, Customer, NULL);

for(int i = 5; i < 9; i++)
	pthread_join(id[i], NULL);

patientCount = 0;
numberOfFreeWRSeats = numberOfFreeSeatsReset;

pthread_create(&id[10], NULL, Dentist, NULL);
for(int i = 11; i < 16; i++)
	pthread_create(&id[i], NULL, Customer, NULL);

for(int i = 10; i < 16; i++)
	pthread_join(id[i], NULL);

return 0;
}

void * Dentist(void * unused)
{

while(dentistHasPatients)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Dentist trying to aquire patient..." << endl;
	pthread_mutex_unlock(&atomicLock);

	sem_wait(&patientReady);

	pthread_mutex_lock(&atomicLock);
		cout << "Dentist trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	
	sem_wait(&seatCountWriteAccess);
		numberOfFreeWRSeats +=1;

		pthread_mutex_lock(&atomicLock);
			cout << "Incremented free seats to " << numberOfFreeWRSeats << endl;	
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

return unused;

}

void * Customer(void * unused)
{

int activeCustomer;

pthread_mutex_lock(&patientCountLock);
	patientCount++;
	activeCustomer = patientCount;
pthread_mutex_unlock(&patientCountLock);

while(patientCount <= totalPatients)
	{
	pthread_mutex_lock(&atomicLock);
		cout << "Customer " << activeCustomer << " trying to aquire seatCountWriteAccess..." << endl;
	pthread_mutex_unlock(&atomicLock);
	sem_wait(&seatCountWriteAccess);
	
		if(numberOfFreeWRSeats > 0)
		{
			numberOfFreeWRSeats -= 1;
			pthread_mutex_lock(&atomicLock);
				cout << "Customer " << activeCustomer << " seated; Remaining chairs = " <<
					numberOfFreeWRSeats << "." << endl;
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
		patientCount++;
		activeCustomer = patientCount;
	pthread_mutex_unlock(&patientCountLock);
	}

dentistHasPatients = false;
return unused;
}
