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

void cond_init(COND *cond)
{
	cond->string = malloc(sizeof(char)*state_length);
}

void cond_copy(COND *to, COND *from)
{
	memcpy(to->string, from->string, sizeof(char)*state_length);
}                              
 
_Bool cond_match(COND *cond, char *state)
{
	for(int i = 0; i < state_length; i++) {
		if(cond->string[i] != DONT_CARE && cond->string[i] != state[i]) {
			cond->m = false;
			return false;
		}
	}
	cond->m = true;
	return true;
}
 
void cond_rand(COND *cond)
{
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_DONTCARE) 
			cond->string[i] = DONT_CARE;
		else {
			if(drand() < 0.5)
				cond->string[i] = '0';
			else
				cond->string[i] = '1';
		}
	}
}

void cond_cover(COND *cond, char *state)
{
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_DONTCARE)
			cond->string[i] = DONT_CARE;
		else
			cond->string[i] = state[i];
	}
}
               
_Bool cond_crossover(COND *cond1, COND *cond2) 
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
		char cc1[state_length];
		char cc2[state_length];
		strncpy(cc1, cond1->string, state_length);
		strncpy(cc2, cond2->string, state_length);
		for(int i = p1; i < p2; i++) { 
			if(cc1[i] != cc2[i]) {
				changed = true;
				char help = cond1->string[i];
				cond1->string[i] = cc2[i];
				cond2->string[i] = help;
			}
		}
		if(changed) {
			strncpy(cond1->string, cc1, state_length);
			strncpy(cond2->string, cc2, state_length);
		}
	}
	return changed;
}
                    
_Bool cond_mutate(COND *cond, char *state)
{
	_Bool mod = false;
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_MUTATION) {
			if(cond->string[i] == DONT_CARE)
				cond->string[i] = state[i];
			else
				cond->string[i] = DONT_CARE;
			mod = true;
		}
	}
	return mod;
}

_Bool cond_general(COND *cond1, COND *cond2)
{
	// returns true if cond1 is more general than cond2
	_Bool gen = false;
	for(int i = 0; i < state_length; i++) {
		if(cond1->string[i] != DONT_CARE && cond1->string[i] != cond2->string[i])
			return false;
		else if(cond1->string[i] != cond2->string[i])
			gen = true;
	}
	return gen;
}
 
_Bool cond_duplicate(COND *cond1, COND *cond2)
{
	for(int i = 0; i < state_length; i++) {
		if(cond1->string[i] != cond2->string[i])
			return false;
	}
	return false;
}

void cond_free(COND *cond)
{
	free(cond->string);
}
 
void cond_print(COND *cond)
{
	for(int i = 0; i < state_length; i++)
		printf("%c", cond->string[i]);
}
