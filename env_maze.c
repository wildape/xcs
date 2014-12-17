/*
 * Copyright (C) 2015 Richard Preen <rpreen@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************
 * Description: 
 **************
 * The discrete maze problem environment module.
 *
 * Reads in the chosen maze from a file where each entry specifies a distinct
 * position in the maze. The maze is toroidal and if the animat reaches one
 * edge it can reenter the maze from the other side. Obstacles are coded as 'O'
 * and 'Q', empty positions as '*', and food as 'F' or 'G'. A 2 bit or 3 bit
 * encoding is automatically chosen depending on the number of perceptions. 8
 * movements are possible to adjacent cells (if not blocked.) The animat is
 * initially placed at a random empty position. The goal is to find the
 * shortest path to the food. 
 *
 * Some mazes require a form of memory to be solved optimally.
 * The optimal average number of steps for each maze is:
 *
 * Woods 1: 1.7
 * Woods 2: 1.7
 * Woods 14: 9.5
 * Maze 4: 3.5
 * Maze 5: 4.61
 * Maze 6: 5.19
 * Maze 7: 4.33
 * Maze 10: 5.11
 * Woods 101: 2.9
 * Woods 101 1/2: 3.1
 * Woods 102: 3.23
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include "cons.h"
#include "random.h"
#include "cl.h"
#include "cl_set.h"
#include "env_maze.h"

#define MAX_PAYOFF 1000.0
const int x_moves[] ={ 0, +1, +1, +1,  0, -1, -1, -1}; 
const int y_moves[] ={-1, -1,  0, +1, +1, +1,  0, -1};

void bin_sensor(char s, char *bin);

char *state;
#ifdef XCSF
double *dstate;
#endif
char maze[50][50];
int xpos;
int ypos;
int xsize;
int ysize;
_Bool reset;

int encoding_bits = 2;

int maze_init(char *filename)
{
	// open maze file
	FILE *file;
	file = fopen(filename, "rt");
	if(file == 0) {
		printf("could not open %s. %s.\n", filename, strerror(errno));
		return EXIT_FAILURE;
	}
	// read maze
	int c; int x = 0; int y = 0;
	while((c = fgetc(file)) != EOF) {
		switch(c) {
 			case '\n':
				y++;
				xsize = x;
				x = 0;
				break;
			case 'Q':
				encoding_bits = 3;
			default:
				maze[y][x] = c;
				x++;
				break;
		}
	}
	ysize = y;
	state_length = 8*encoding_bits;
	state = malloc(sizeof(char)*state_length);
	num_actions = 8;
	pred_length = 8;
	multi_step = true;
	max_payoff = MAX_PAYOFF;
#ifdef XCSF
	dstate = malloc(sizeof(double)*pred_length);
#endif
	printf("Loaded MAZE = %s\n", filename);
	return EXIT_SUCCESS;
}

void maze_rand_pos()
{
	reset = false;
	do {
		xpos = irand(0,xsize);
		ypos = irand(0,ysize);
	} while(maze[ypos][xpos] != '*');
}

_Bool maze_isreset()
{
	return reset;
}

char *maze_state()
{
	int spos = 0;
	for(int x = -1; x < 2; x++) {
		for(int y = -1; y < 2; y++) {
			// ignore current pos
			if(x == 0 && y == 0)
				continue;
			// toroidal maze
			char s = maze[(ysize-(ypos+y))%ysize][(xsize-(xpos+x))%xsize];
			// convert sensor to binary
			char b[3];
			bin_sensor(s, b);
			for(int i = 0; i < encoding_bits; i++) {
				state[spos] = b[i];
				spos++;
			}
		}
	}
	return state;
}

#ifdef XCSF
double *maze_dstate()
{
	double tmp;
	// convert binary sensors to decimal
	for(int i = 0; i < state_length; i+=encoding_bits) {
		dstate[i/encoding_bits] = 0.0;
		for(int j = 0; j < encoding_bits; j++) {
			tmp = (double)(state[i+j] - '0');
			if(tmp > 0.0)
				dstate[i/encoding_bits] += tmp+(tmp*pow(j,2));
		}
	}
	// scale between [-1,1]
	for(int i = 0; i < pred_length; i++)
		dstate[i] = (dstate[i]/((pow(encoding_bits,2)-1.0)/2.0))-1.0;
	return dstate;
}
#endif

void bin_sensor(char s, char *bin)
{
	switch(s) {
		case '*':
			strncpy(bin, "000", 3);
			break;
		case 'O':
			strncpy(bin, "010", 3);
			break;
		case 'G':
			strncpy(bin, "111", 3);
			break;
		case 'F':
			strncpy(bin, "110", 3);
			break;
		case 'Q':
			strncpy(bin, "011", 3);
			break;    
		default :
			printf("unsupported maze state\n");
			exit(EXIT_FAILURE);
	}
}

double maze_execute(int move)
{
	if(move < 0 || move > 7) {
		printf("invalid maze action\n");
		exit(EXIT_FAILURE);
	}
	// toroidal maze
	int newx = (xsize-(xpos+x_moves[move]))%xsize;
	int newy = (ysize-(ypos+y_moves[move]))%ysize;
	// make the move and recieve reward
	switch(maze[newy][newx]) {
		case '*':
			ypos = newy;
			xpos = newx;
			reset = false;
			return 0.0;
		case 'F': 
		case 'G':
			ypos = newy;
			xpos = newx;
			reset = true;
			return MAX_PAYOFF;
		case 'O': 
		case 'Q':
			reset = false;
			return 0.0;
		default:
			printf("invalid maze type\n");
			exit(EXIT_FAILURE);
	}
}         
