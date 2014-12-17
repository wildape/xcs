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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "cons.h"
#include "random.h"
#include "env_mux.h"

#define MAX_PAYOFF 1000.0

int pos_bits;
char *state;
#ifdef XCSF
double *dstate;
#endif

void mux_init(int bits)
{
	pred_length = bits;
	state_length = bits;
	state = malloc(sizeof(char)*state_length);
	num_actions = 2;
	multi_step = false;
	max_payoff = 1000.0;
#ifdef XCSF
	dstate = malloc(sizeof(double)*pred_length);
#endif
	for(pos_bits = 1.0; pos_bits+pow(2.0,pos_bits) <= state_length; pos_bits++);
	pos_bits--;
}

char *mux_state()
{
	for (int i = 0; i < state_length; i++) {
		if (drand() < 0.5)
			state[i] = '0';
		else
			state[i] = '1';
	}
	return state;
}

#ifdef XCSF
double *mux_dstate()
{
	for(int i = 0; i < state_length; i++) {
		if(state[i] == '0')
			dstate[i] = -1.0;
		else
			dstate[i] = 1.0;
	}
	return dstate;
}
#endif      

double mux_execute(int act)
{
	int pos = pos_bits;
	for (int i = 0; i < pos_bits; i++) {
		if (state[i] == '1')
			pos += pow(2.0, (double)(pos_bits-1-i));
	}
	int answer;
	for(int i = 31; i >= 0; i--) {
		if((state[pos] & (1 << i)) != 0)
			answer = 1;
		else
			answer = 0;
	}
	if (act == answer)
		return MAX_PAYOFF;
	else
		return 0.0;
}  
