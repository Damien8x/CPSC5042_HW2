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

int numberOfFreeWRSeats;
int patientCount;

int main(){

sem_init(&dentistReady, 0, 0);
sem_init(&seatCountWriteAccess, 0, 1);
sem_init(&patientReady, 0, 0);

string strSeatCount;
string strPatientCount;



cout << "Enter Number of Seats: " << endl;
cin >> strSeatCount;
cout << "Enter Number of Patients: " << endl;
cin >> strPatientCount;

numberOfFreeWRSeats = stoi(strSeatCount);
patientCount = stoi(strPatientCount);


return 0;
}

void * Dentist(void * unused)
{
bool run = true;
while(run)
	{
	pthread_cond_wait(&atomic)
		cout << "Dentist trying to aquire patient..." << endl;
	pthread_cond_signal(&atomic);

	sem_wait(&patientReady);

	pthread_cond_wait(&atomic);
		cout << "Dentist trying to aquire seatCountWriteAccess..." << endl;
	pthread_cond_signal(&atomic);
	
	sem_wait(&seatCountWriteAccess);
		numberOfFreeWRSeats +=1;

		pthread_cond_wait(&atomic);
			cout << "Incremented free seats to " << numberOfFreeWRSeats << endl;	
		pthread_cond_signal(&atomic);

	pthread_cond_wait(&atomic);
		cout << "Dentist ready to consult..." << endl;
	pthread_cond_signal(&atomic);
	
	sem_post(&dentistReady);
	
	pthread_cond_wait(&atomic);
		cout << "Dentist releasing seatCountWriteAccess..." << endl;
	pthread_cond_signal(&atomic);
	
	sem_post(&seatCountWriteAccess);

	pthread_cond_wait(&atomic);
		cout << "Dentist consulting patient" << endl;
	pthread_cond_signal(&atomic);
	}



}

void * Customer(void * unused)
{
bool run = true;
while(run)
	{
	sem_wait(&seatCountWriteAccess);
	if(numberOfFreeWRSeats > 0)
	{
		numberOfFreeWRSeats -= 1;
		sem_post(&patientReady);
		sem_post(&seatCountWriteAccess);
		sem_wait(&dentistReady);
	}
	else
	{
		sem_post(&seatCountWriteAccess);
	
	}
	}
}
