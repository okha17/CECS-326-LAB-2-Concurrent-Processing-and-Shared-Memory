#include <stdio.h> // For output functions
#include <unistd.h> // Access POSIX OS API
#include <stdlib.h> // Used for srand and exit
#include <sys/shm.h> // Library for shared memory functions
#include <sys/ipc.h> // Header file for Interproces communication
#include <sys/types.h> //Header file for pid_t data type 
#include <signal.h> // Header file used for signals

void move_left(char(*)[10][10]); // Function that moves the fish left 
void move_right(char(*)[10][10]); // Function that moves the fish right
int location(char(*)[10][10]); // Function that checks current location of fish 
void find_pellet(char (*)[10][10]); // Function that searches for pellet and moves fish 
void Interrupt(); // Function that interrupts the process 
void Terminate(); // Function that termiantes the process

char (*mill)[10][10];  // char pointer to 2D array used as shared memory 
int shm_id; // int value the stores shared memory ID

int main(int argc, char *argv[])
{
	signal(SIGINT, Interrupt); //Signal for interrupt 
	signal(SIGTERM, Terminate); //Signal for termination
 
	// Generate unique key used for shared memory
	// Pass in the SwimMill file and a random char
	key_t key = ftok("SwimMill.c", 'a');
	// Shmget is used to return identifier for shared memory
	// Unique key, size of 2D array, and IPC_CREAT used to 
	// create new memory segment for shared memory while 
	// 0666 is sets access permission for shmflag parameter  
	int shm_id = shmget(key, sizeof(char[10][10]), IPC_CREAT|0666);
	
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

	*mill[9][4] = 'F'; // Fish set in the middle of the last row 
	
	//Have in constant loop looking for pellets and sleeping
	while(1)
	{	
		sleep(1);
		find_pellet(mill);
	}
	return 0;
}

void move_left(char (* stream)[10][10])
{
	int row = 9;
	int col = 0;
	//For loop that locates fish
	for(int j = 0; j < 10; j++)
	{
		if(*stream[row][j] == 'F')
		{
			col = j;
		}
	}
	// If the fish isn't on the left edge of the mill
	// it moves left 
	if(col != 0)
	{
		*stream[row][col] = '~';
		col--;
		*stream[row][col] = 'F';
	}
}

void move_right(char (* stream)[10][10])
{
	int row = 9;
	int col = 0;
	// For loop that locates fish
	for(int j = 0; j < 10; j++)
	{
		if(*stream[row][j] == 'F')
		{
			col = j;
		}
	}
	// If the fish isn't on the right edge of the mill
	// it moves right 
	if (col != 9 )
	{
		*stream[row][col] = '~';
		col++;
		*stream[row][col] = 'F';
	}
	
}

int location(char (* stream)[10][10])
{
	int row = 9;
	// For loop that finds current coordinate of the fish 
	// and returns the column it is at 
	for(int j = 0; j < 10; j++)
	{
		if(*stream[row][j] == 'F')
		{
			return j;
		}
	}
}

void find_pellet(char (* stream)[10][10])
{
	int pelletrow = 0;
	int pelletcol = 0;
	// For loop that finds where a pellet is at the moment
	// Starts at [9][9] to find where the closest pellet may be 
	for(int i = 9; i >= 0; i--)
	{
		for(int j = 9; j >= 0; j--)
		{
			
			if(*stream[i][j] == 'P')
			{
				pelletcol = j;
				pelletrow = i;
			}
		}
	}
	
	printf("\nPellet location: (%d,%d)\n", pelletrow,pelletcol);
	// If conditional to check if the row and column value 
	// in the 2D array returns a pellet 
	if(*stream[pelletrow][pelletcol] == 'P')
	{
		printf("\nFish location: (%d,%d)\n", 9,location(stream));
		
		// Takes the difference of the fish's column and the
		// column the pellet is in and stores it in move
		int move = pelletcol - location(stream);
		
		// 4 - 7 = -3 so we should move right
		// 7 - 4 = +3 so we should move left
		
		// Loops until the fish moves to the same column as the pellet 
		while(move != 0)
		{
			// if the value of move is negative then the fish
			// must move left to find the pellet 
			if( move < 0)
			{
				move_left(stream); // moves the fish left
				move++; 
				sleep(1); // Sleep to have 1 sec refresh rate for movement 
			}
			else
			{
				move_right(stream); // moves the fish right
				move--;
				sleep(1); // Sleep to have 1 sec refresh rate for movement
			}
		}	
	}
}

void Interrupt()
{
	printf("Interupt on Fish!\n");
	shmdt(mill); // Detach from shared memory
	exit(0); // Exit the process
}

void Terminate()
{
	printf("Termiate Fish!\n");
	shmdt(mill); // Detach from shared memory
	exit(0);  // Exit the process
}

