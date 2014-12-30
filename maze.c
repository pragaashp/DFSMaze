#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

/*
 * Returns 0 or 1 to check presence of wall.
 * Bit Representation: 00|00|00|00 --> North,East,South,West
 * Input: 0,1,2,3 for North,East,South,West
 */
uint8_t __checkWall(uint8_t *maze, int dir)
{
	uint8_t ext = 0x40;
	return (*maze & (ext >> (dir * 2)));
}

void __printMaze(uint8_t *maze, int width, int height)
{
	#pragma omp parallel for
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			printf("-");
			char *north = (__checkWall(&maze[width*y + x],0))? "---":"   ";
			printf("%s", north);
		}
		printf("-");
		printf("\n");
		for (int x = 0; x < width; ++x)
		{
			char *west = (__checkWall(&maze[width*y + x],3))? "|":" ";
			printf("%s   ", west);
		}
		char *east = (__checkWall(&maze[width*(y + 1) - 1],1))? "|":" ";
		printf("%s\n", east);
		if (y == height - 1)
		{
			for (int x = 0; x < width; ++x)
			{
				char *south = (__checkWall(&maze[width*y + x],2))? "---":"   ";
				printf("-");
				printf("%s", south);
			}
			printf("-\n");
		}
	}
}

int __checkVisited(uint8_t *maze, int *list)
{
	int *ptr = list;
	while(ptr < list + 4)
	{
		if ((*ptr != -1) && ((*(maze + ptr[0])) == 0x55))
		{
			if (ptr != list)
			{
				*list = *list ^ *ptr;
				*ptr = *list ^ *ptr;
				*list = *ptr ^ *list;
			}
			return 1;
		}
		++ptr;
	}
	return 0;
}

void __clearWall(uint8_t *maze, int cell, int neighbor, int width)
{
	int dir = neighbor - cell;
	if (!(dir + width))						//NEIGHBOR AT NORTH
	{
		*(maze + neighbor) &= 0xF3;
		*(maze + cell) &= 0x3F;
	}
	else if (!(dir-width))					//NEIGHBOR AT SOUTH
	{				
		*(maze + neighbor) &= 0x3F;
		*(maze + cell) &= 0xF3;
	}
	else if (!(dir+1))						//NEIGHBOR AT WEST
	{
		*(maze + neighbor) &= 0xCF;
		*(maze + cell) &= 0xFC;
	}
	else if (!(dir-1))						//NEIGHBOR AT EAST
	{
		*(maze + neighbor) &= 0xFC;
		*(maze + cell) &= 0xCF;
	}
}

/*
 * Returns a randomly permuted list of valid neighbors of a cell in the maze.
 */
int* __findNeighbours(int i, int width, int height)
{
	int *neighbors = (int *) malloc(4*sizeof(int));
	int r = rand() % 4;
	neighbors[r] = (i < width)? (-1):(i-width);									//NORTH
	neighbors[(r+1) % 4] = (i % width == width-1)? (-1):(i+1);					//EAST
	neighbors[(r+2) % 4] = (i / width == height-1)? (-1):(i+width);	//SOUTH
	neighbors[(r+3) % 4] = (i % width)? (i-1):(-1);					//WEST
	return neighbors;
}

void __generateMaze(uint8_t *maze, int width, int height, int difficulty)
{
	int unvisitedCells = width*height;
	int *stack = (int *) calloc(unvisitedCells,sizeof(int));
	int *sp = stack;
	int numExit = 0;
	// Initialization
	srand(time(NULL));
	int initRow = rand() % height;
	int start = initRow * width;
	*(maze + start) ^= 0x01;
	--unvisitedCells;

	int curr = start;
	while (unvisitedCells)
	{
		//printf("\nunvisitedCells: %d\n", unvisitedCells);
		//__printMaze(maze,width,height);
		int *list = __findNeighbours(curr, width, height);
		//printf("list[0] Before: %d, Cell Value: %d\n", list[0],*(maze + list[0]));
		if (__checkVisited(maze,list))
		{
			//printf("list[0] After: %d, Cell Value: %d\n", list[0],*(maze + list[0]));
			__clearWall(maze,curr,list[0],width);
			curr = list[0];
			*sp = curr;
			++sp;
			--unvisitedCells;
			if (curr % width == width-1)
			{
				++numExit;
				if (numExit == difficulty)
				{
					*(maze + curr) ^= 0x10;
				}
			}
		}
		else
		{
			--sp;
			curr = *sp;
			*sp = 0x00;
		}
	}
}

int main()
{	
	clock_t initTime = clock();
	clock_t endTime;
	int width, height, difficulty;
	char printReq = '\0';
	double totalTime;
	printf("ENTER Maze Width: ");
	scanf("%d", &width);
	printf("ENTER Maze Height: ");
	scanf("%d", &height);
	printf("ENTER Maze Difficulty (Max: %d): ", height);
	scanf("%d", &difficulty);

	uint8_t *maze_map = (uint8_t *) calloc(width*height,sizeof(uint8_t));
	if (!maze_map)
	{
		printf("Error Initializing Maze!\n");
		exit(-1);
	}
	memset(maze_map,0x55,width*height);

	__generateMaze(maze_map, width, height, difficulty);

	endTime = clock() - initTime;
	totalTime = ((double)endTime)/(CLOCKS_PER_SEC/1000);
	printf("\nMaze Generation completed in %.3f millis.\n\n", totalTime);

	printf("Print Maze (y/n): ");
	scanf(" %c", &printReq);
	printf("\n");

	if (printReq == 'y')
	{
		__printMaze(maze_map, width, height);
	}

	return 0;
}