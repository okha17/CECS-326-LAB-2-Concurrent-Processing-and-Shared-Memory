#include <stdio.h> // For output functions
#include <unistd.h> // Access POSIX OS API
#include <stdlib.h> // Used for srand and exit
#include <sys/shm.h> // Library for shared memory functions
#include <sys/ipc.h> // Header file for Interproces communication
#include <sys/types.h> //Header file for pid_t data type 
#include <signal.h> // Header file used for signals
#include <sys/wait.h> // Header file used for wait function 


void Interrupt();
void Terminate(); 

pid_t fish, pellet; // Used for forking for fish and pellet process
char (*mill)[10][10];  // char pointer to 2D array used as shared memory 
int shm_id; // int value the stores shared memory ID

int main(int argc, char *argv[])
{
	signal(SIGINT, Interrupt); //Signal for interrupt 
	
	// Generate unique key used for shared memory
	key_t key = ftok("SwimMill.c", 'a');
	// Shmget is used to return identifier for shared memory
	// Unique key, size of 2D array, and IPC_CREAT used to 
	// create new memory segment for shared memory while 
	// 0666 is sets access permission for shmflag parameter  
	shm_id = shmget(key, sizeof(char[10][10]), IPC_CREAT|0666);
	
	// Check to make sure shared ID is valid
	if(shm_id < 0)
	{
		perror("Failed to generate memory ID\n");
		exit(1);
	} 
	
	// Attach shared memory before you can use the segment 
	// Pass in shared memory ID 
	mill = (char(*)[10][10])shmat(shm_id, NULL, 0);
	
	// Check to make sure attach did not fail 
	if((mill = (char(*)[10][10])shmat(shm_id, NULL, 0))  < (char(*)[10][10]) 0)
	{
		perror("Failed to attach memory!\n");
		exit(1);
	}
	
	// Set 2D array to tilde value to display empty swim mill 
	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			*mill[i][j] = '~';
		}
	}	
	
	// Fork process 
	fish = fork();
	
	// Error occured 
	if(fish == -1)
	{
		perror("Fork failed!\n");
		exit(1);
	}
	
	//Child process created
	else if(fish == 0)
	{
		// Replace core image with Fish
		execv("Fish", argv);
	}
	
	// Parent Process
	else	
	{	
		// Loop for 30 iterations and sleep for 30 second run time
		for(int i = 0; i < 30; i++)
		{
			pellet = fork();
			//Error occured
			if(pellet == -1)
			{
				perror("Fork failed!\n");
				exit(1);
			}
			//Child process created
			else if(pellet == 0)
			{
				// Replace core image with Pellet
				execv("Pellet", argv);
				
			}
			// Parent process 
			else
			{
				// Display 2D array to show movements
				printf("\n");
				for(int i = 0; i < 10; i++)
				{
					for(int j = 0; j < 10; j++)
					{
						printf("%c", *mill[i][j]);
					}
				printf("\n");
				}
				sleep(1); //Sleep in order to run display for 30 seconds
			}	
		}
	}
	Terminate();
	return 0;
}

void Interrupt()
{
	printf("\nInterupt on SwimMill!\n");
	kill(fish, SIGINT); // Kill the child process
	kill(pellet, SIGINT); // Kill the child process
	shmdt(mill); // Detach from shared memory
	shmctl(shm_id, IPC_RMID, NULL); // Delete the shared memory
	exit(0); // Exit the program
}


void Terminate()
{
	printf("\nEnd of SwimMill!\n");
	kill(fish, SIGTERM); // Kill the child process
	kill(pellet, SIGTERM); // Kill the child process
	shmdt(mill); // Detach from shared memory
	shmctl(shm_id, IPC_RMID, NULL);  // Delete the shared memory
	
	exit(0); // Exit the program
}

