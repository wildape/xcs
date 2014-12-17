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
 * Classifier ternary [0,1,#] conditions module.
 *
 * A classifier matches for a binary environment state if, and only if, all
 * bits in the environment string match the condition with the exception of the
 * DONT_CARE symbol, which matches a logical '1' or '0' for that bit.  Provides
 * functions to generate random or matching conditions, to mutate a condition,
 * and print it, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "cons.h"
#include "random.h"
#include "cl.h"

_Bool mutate_con(CL *c, char *state);

void cond_init(CL *c)
{
	c->con = malloc(sizeof(char)*state_length);
}

void cond_copy(CL *to, CL *from)
{
	memcpy(to->con, from->con, sizeof(char)*state_length);
}                              
 
_Bool cond_match(CL *c, char *state)
{
	for(int i = 0; i < state_length; i++) {
		if(c->con[i] != DONT_CARE && c->con[i] != state[i])
			return false;
	}
	return true;
}
 
void cond_rand(CL *c)
{
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_DONTCARE) 
			c->con[i] = DONT_CARE;
		else {
			if(drand() < 0.5)
				c->con[i] = '0';
			else
				c->con[i] = '1';
		}
	}
}

void cond_cover(CL *c, char *state)
{
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_DONTCARE)
			c->con[i] = DONT_CARE;
		else
			c->con[i] = state[i];
	}
}
               
_Bool cond_crossover(CL *c1, CL *c2) 
{
	// two point crossover
	_Bool changed = false;
	if(drand() < P_CROSSOVER) {
		int p1 = irand(0, state_length);
		int p2 = irand(0, state_length)+1;
		if(p1 > p2) {
			int help = p1;
			p1 = p2;
			p2 = help;
		}
		else if(p1 == p2) {
			p2++;
		}
		char cond1[state_length];
		char cond2[state_length];
		strncpy(cond1, c1->con, state_length);
		strncpy(cond2, c2->con, state_length);
		for(int i = p1; i < p2; i++) { 
			if(cond1[i] != cond2[i]) {
				changed = true;
				char help = c1->con[i];
				c1->con[i] = cond2[i];
				c2->con[i] = help;
			}
		}
		if(changed) {
			strncpy(c1->con, cond1, state_length);
			strncpy(c2->con, cond2, state_length);
		}
	}
	return changed;
}
                    
_Bool cond_mutate(CL *c, char *state)
{
	_Bool mod = false;
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_MUTATION) {
			if(c->con[i] == DONT_CARE)
				c->con[i] = state[i];
			else
				c->con[i] = DONT_CARE;
			mod = true;
		}
	}
	return mod;
}

_Bool cond_general(CL *c1, CL *c2)
{
	// returns true if c1 is more general than c2
	_Bool gen = false;
	for(int i = 0; i < state_length; i++) {
		if(c1->con[i] != DONT_CARE && c1->con[i] != c2->con[i])
			return false;
		else if(c1->con[i] != c2->con[i])
			gen = true;
	}
	return gen;
}
 
_Bool cond_duplicate(CL *c1, CL *c2)
{
	if(strcmp(c1->con, c2->con) == 0)
		return true;
	else
		return false;
}

void cond_free(CL *c)
{
	free(c->con);
}
 
void cond_print(CL *c)
{
	for(int i = 0; i < state_length; i++)
		printf("%c", c->con[i]);
}
