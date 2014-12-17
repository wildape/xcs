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
 * The prediction array module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cons.h"
#include "random.h"
#include "cl.h"
#include "cl_set.h"
#include "pa.h"

double *pa;
double *nr; 

#ifdef XCSF
void pa_init(NODE **set, double *state)
#else
void pa_init(NODE **set)
#endif
{
	if(pa == NULL) {
		pa = malloc(sizeof(double)*num_actions);
		nr = malloc(sizeof(double)*num_actions);
	}
	for(int i = 0; i < num_actions; i++) {
		pa[i] = 0.0;
		nr[i] = 0.0;
	}
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		CL *c = iter->cl;
#ifdef XCSF
		pa[c->act] += pred_compute(c, state) * c->fit;
#else
		pa[c->act] += c->pre * c->fit;
#endif
		nr[c->act] += c->fit;
	}
	for(int i = 0; i < num_actions; i++) {
		if(nr[i] != 0.0)
			pa[i] /= nr[i];
		else
			pa[i] = 0.0;
	}
}

int pa_best_action()
{
	int action = 0;
	for(int i = 1; i < num_actions; i++) {
		if(pa[action] < pa[i])
			action = i;
	}
	return action;
}

int pa_rand_action()
{
	int action = 0;
	do {
		action = irand(0, num_actions);
	} while(nr[action] == 0);
	return action;
}

double pa_best_val()
{
	double max = pa[0];
	for(int i = 1; i < num_actions; i++) {
		if(max < pa[i])
			max = pa[i];
	}
	return max;
}

double pa_val(int act)
{
	if(act >= 0 && act < num_actions)
		return pa[act];
	return -1.0;
}
