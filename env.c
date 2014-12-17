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
 * The problem environment module.
 *
 * Initialises the problem environment and provides abstracted functions to
 * perceive the environment state and to execute actions and receive reward.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "cons.h"
#include "random.h"
#include "env_mux.h"
#include "env_maze.h"

int env;
#define MUX 0
#define MAZE 1

void env_init(char **argv)
{
	if(strcmp(argv[1], "mp") == 0) {
		env = MUX;
	}
	else if(strcmp(argv[1], "maze") == 0) {
		env = MAZE;
	}
	else {
		printf("invalid env: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	switch(env) {
		case MUX:
			mux_init(atoi(argv[2]));
			break;
		case MAZE:
			maze_init(argv[2]);
			break;
	}
}

void env_reset()
{
	switch(env) {
		case MAZE:
			maze_rand_pos();
			break;
	}
}

double env_exec_action(int action)
{
	switch(env) {
		case MUX:
			return mux_execute(action);
		case MAZE:
			return maze_execute(action);
	}
	exit(EXIT_FAILURE);
}

char *env_get_state()
{
	switch(env) {
		case MUX:
			return mux_state();
		case MAZE:
			return maze_state(); 
	}
	exit(EXIT_FAILURE);
}

#ifdef XCSF
double *env_get_dstate()
{
	switch(env) {
		case MUX:
			return mux_dstate();
		case MAZE:
			return maze_dstate(); 
	}
	exit(EXIT_FAILURE);
}
#endif

_Bool env_is_reset()
{
	switch(env) {
		case MUX:
			return true;
		case MAZE:
			return maze_isreset(); 
	}
	exit(EXIT_FAILURE);
}
