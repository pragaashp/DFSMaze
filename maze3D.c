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

/*
 * Type: 0 (/), 1 (\)
 */
uint8_t __checkStairs(uint8_t *maze, int type)
{
	uint8_t mask = 0x80;
	return (*maze & (mask >> (type * 2)));
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
			printf("%s ", west);
			char stairs;
			uint8_t upstairs = __checkStairs(&maze[y*width + x],0);
			uint8_t downstairs = __checkStairs(&maze[y*width + x],1);
			if (upstairs && downstairs)
			{
				stairs = 'X';
			}
			else if (upstairs)
			{
				stairs = '/';
			}
			else if (downstairs)
			{
				stairs = '\\';
			}
			else
			{
				stairs = ' ';
			}
			printf("%c ", stairs);
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

int __checkVisited(uint8_t *maze0, uint8_t *maze1, uint8_t *maze2, int *list, int size)
{
	int *ptr = list;
	while(ptr < list + 6)
	{
		if (*ptr != -1)
		{
			int z = *ptr % size;
			if ((*ptr < size && ((*(maze0 + z)) == 0x55)) || ((*ptr >= size && *ptr < 2*size) && ((*(maze1 + z)) == 0x55)) || (*ptr >= 2*size && ((*(maze2 + z)) == 0x55)))
			{
				if (ptr != list)
				{
					*list = *list ^ *ptr;
					*ptr = *list ^ *ptr;
					*list = *ptr ^ *list;
				}
				return 1;
			}
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
		*(maze + neighbor) &= 0xFB;
		*(maze + cell) &= 0xBF;
	}
	else if (!(dir-width))					//NEIGHBOR AT SOUTH
	{				
		*(maze + neighbor) &= 0xBF;
		*(maze + cell) &= 0xFB;
	}
	else if (!(dir+1))						//NEIGHBOR AT WEST
	{
		*(maze + neighbor) &= 0xEF;
		*(maze + cell) &= 0xFE;
	}
	else if (!(dir-1))						//NEIGHBOR AT EAST
	{
		*(maze + neighbor) &= 0xFE;
		*(maze + cell) &= 0xEF;
	}
}

void __createStairs(uint8_t *maze_cell, uint8_t *maze_neighbor, int cell, int neighbor, int size)
{
	if (neighbor > cell)
	{
		*(maze_cell + (cell % size)) |= 0x80;
		*(maze_neighbor + (neighbor % size)) |= 0x20;
	}
	else
	{
		*(maze_cell + (cell % size)) |= 0x20;
		*(maze_neighbor + (neighbor % size)) |= 0x80;
	}
}

/*
 * Returns a randomly permuted list of valid neighbors of a cell in the maze.
 */
int* __findNeighbours(int i, int width, int height, int size)
{
	int *neighbors = (int *) malloc(6*sizeof(int));
	int z = i % size;
	int r = rand() % 6;
	neighbors[r] = (z < width)? (-1):(i-width);						//NORTH
	neighbors[(r+1) % 6] = (z % width == width-1)? (-1):(i+1);		//EAST
	neighbors[(r+2) % 6] = (z / width == height-1)? (-1):(i+width);	//SOUTH
	neighbors[(r+3) % 6] = (z % width)? (i-1):(-1);					//WEST
	neighbors[(r+4) % 6] = (i == z)? (-1):(i-size);					//BOTTOM
	neighbors[(r+5) % 6] = (i >= 2*size)? (-1):(i+size);			//TOP
	return neighbors;
}

void __generateMaze(uint8_t *maze0, uint8_t *maze1, uint8_t *maze2, int width, int height, int difficulty, int size)
{
	int unvisitedCells = 3*size;
	int *stack = (int *) calloc(unvisitedCells,sizeof(int));
	int *sp = stack;
	int numExit = 0;
	// Initialization
	srand(time(NULL));
	int initRow = rand() % height;
	int start = initRow * width;
	*(maze0 + start) ^= 0x01;
	--unvisitedCells;

	int curr = start;
	while (unvisitedCells)
	{
		int *list = __findNeighbours(curr, width, height, size);
		if (__checkVisited(maze0, maze1, maze2, list, size))
		{
			if (list[0] - curr == size)
			{
				if (curr < size)
				{
					__createStairs(maze0, maze1, curr, list[0], size);
				}
				else
				{
					__createStairs(maze1, maze2, curr, list[0], size);
				}
			}
			else if (curr - list[0] == size)
			{
				if (curr < 2*size)
				{
					__createStairs(maze1, maze0, curr, list[0], size);	
				}
				else
				{
					__createStairs(maze2, maze1, curr, list[0], size);
				}
			}
			else
			{
				if (curr < size)
				{
					__clearWall(maze0,curr,list[0],width);
				}
				else if (curr < 2*size)
				{
					__clearWall(maze1,(curr % size),(list[0] % size),width);
				}
				else
				{
					__clearWall(maze2,(curr % size),(list[0] % size),width);
				}
			}
			curr = list[0];
			*sp = curr;
			++sp;
			--unvisitedCells;
			if ((curr >= 2*size) && ((curr % size) % width == width-1))
			{
				++numExit;
				if (numExit == difficulty)
				{
					*(maze2 + (curr % size)) ^= 0x10;
				}
			}
		}
		else
		{
			--sp;
			curr = *sp;
			*sp = -1;
		}
	}
}

int main()
{	
	clock_t initTime = clock();
	clock_t endTime;
	int width, height, difficulty, size;
	char printReq = '\0';
	double totalTime;
	printf("ENTER Maze Width: ");
	scanf("%d", &width);
	printf("ENTER Maze Height: ");
	scanf("%d", &height);
	printf("ENTER Maze Difficulty (Max: %d): ", height);
	scanf("%d", &difficulty);

	size = width * height;

	uint8_t *maze_map_lvl0 = (uint8_t *) calloc(size,sizeof(uint8_t));
	uint8_t *maze_map_lvl1 = (uint8_t *) calloc(size,sizeof(uint8_t));
	uint8_t *maze_map_lvl2 = (uint8_t *) calloc(size,sizeof(uint8_t));
	if (!maze_map_lvl0 || !maze_map_lvl1 || !maze_map_lvl2)
	{
		printf("Error Initializing Maze!\n");
		exit(-1);
	}
	memset(maze_map_lvl0,0x55,size);
	memset(maze_map_lvl1,0x55,size);
	memset(maze_map_lvl2,0x55,size);

	__generateMaze(maze_map_lvl0, maze_map_lvl1, maze_map_lvl2, width, height, difficulty, size);

	endTime = clock() - initTime;
	totalTime = ((double)endTime)/(CLOCKS_PER_SEC/1000);
	printf("\nMaze Generation completed in %.3f millis.\n\n", totalTime);

	printf("Print Maze (y/n): ");
	scanf(" %c", &printReq);
	printf("\n");

	if (printReq == 'y')
	{
		__printMaze(maze_map_lvl0, width, height);
		__printMaze(maze_map_lvl1, width, height);
		__printMaze(maze_map_lvl2, width, height);
		printf("\nNote: [ / ] is UPSTAIRS-ONLY, \n      [ \\ ] is DOWNSTAIRS-ONLY \n      [ X ] is TWO-WAY.\n");
	}

	return 0;
}