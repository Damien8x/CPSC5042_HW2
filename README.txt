//Damien Sudol
//HW3 README
//Version 1.0

*********************************************************************************

				LazyDentistit.cpp

The LazyDentist program is a simulation of the Lazy Dentist problem, solved
through the use of semaphores, ensuring all aspects of the program are 
executed with a logic determining the ordering of exectuion and how
threads are communicating within the program. 

LazyDentist will accept two command line arguments, both integer values:
	first argument)   Waiting room seat count
	second argument)  Number of Customer


These arguments will influence how many threads the program utilizes,
number of Customers == number of Customer threads.

All output shares a single lock, ensuring complete output, used for 
debugging and verification. Additonal locks have been added to
threads to ensure all operations are completed atomically, preventing
any possible race conditions. 



*********************************************************************************

			Compiling LazyDentist.cpp

Follow the provided command line instruction for compiling. Instruction is
designed to be entered from within the CWD containing LazyDentist.cpp.
Command:
	g++ -std=c++11 LazyDentist.cpp -lpthread


			Running LazyDentist.cpp

After succesfully compiling LazyDentist.cpp use the provided command line input
to ensure proper execution from within the CWD containing LazyDentist.cpp:
Command:
	./a.out [seat count] [patient count]

Example:
	./a.out 4 6

Provided example will execute LazyDentist program with 4 waiting room seats
and 6 patients.

			Exiting LazyDentist.cpp

Program contains intended infinite loops and will not voluntarily exit. To exit
use the provided command.
Command:
	CTRL c

*****************************Important Notes*************************************

Command line input is designed to be positive integer values.
Any values entered which are not positive integers will result in undesired
behavior and possible crashing of the program.

Program contains intended infinite loops. Program is not desined to voluntarily 
exit. User must manually exit program. Refer to above, "Exiting LazyDentist.cpp"

*********************************************************************************


	 
