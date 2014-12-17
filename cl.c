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
#include "cl.h"

_Bool mutate_act(CL *c);

void cl_init(CL *c, int size, int time)
{
	cond_init(c);
	c->fit = INIT_FITNESS;
	c->err = INIT_ERROR;
	c->num = 1;
	c->exp = 0;
	c->size = size;
	c->time = time;
	pred_init(c);
#ifdef SELF_ADAPT_MUTATION
	sam_init(c);
#endif
}

void cl_copy(CL *to, CL *from)
{
	cl_init(to, from->size, from->time);
	cond_copy(to, from);
	to->act = from->act;
	pred_copy(to, from);
#ifdef SELF_ADAPT_MUTATION
	sam_copy(to, from);
#endif
}

void rand_act(CL *c)
{
	c->act = irand(0, num_actions);
}

_Bool mutate(CL *c, char *state)
{
#ifdef SELF_ADAPT_MUTATION
	sam_adapt(c);
	P_MUTATION = c->mu[0];
#endif
	_Bool mod = cond_mutate(c, state);
	if(mutate_act(c))
		mod = true;
	return mod;
}

_Bool mutate_act(CL *c)
{
	_Bool mod = false;
	if(drand() < P_MUTATION) {
		int act = 0;
		do {
			act = irand(0, num_actions);
		} while(act == c->act);
		c->act = act;
		mod = true;
	}
	return mod;
}

_Bool cl_duplicate(CL *c1, CL *c2)
{
	if(strcmp(c1->con, c2->con) == 0 && c1->act == c2->act)
		return true;
	else
		return false;
}

_Bool cl_subsumes(CL *c1, CL *c2)
{
	if(c1->act == c2->act && c1->exp > THETA_SUB && c1->err < EPS_0)
		if(cl_general(c1, c2))
			return true;
	return false;
}

_Bool cl_subsumer(CL *c)
{
	if(c->exp > THETA_SUB && c->err < EPS_0)
		return true;
	else
		return false;
}

_Bool cl_general(CL *c1, CL *c2)
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

double cl_del_vote(CL *c, double avg_fit)
{
	if(c->fit / c->num >= DELTA * avg_fit || c->exp < THETA_DEL)
		return c->size * c->num;
	return c->size * c->num * avg_fit / (c->fit / c->num); 
}

 
#ifdef XCSF
double cl_update_err(CL *c, double p, double *state)
{
	double pre = pred_compute(c, state);
	if(c->exp < 1.0/BETA) 
		c->err = (c->err * (c->exp-1.0) + fabs(p - pre)) / (double)c->exp;
	else
		c->err += BETA * (fabs(p - pre) - c->err);
	return c->err * c->num;
}
 
#else
double cl_update_err(CL *c, double p)
{
	if(c->exp < 1.0/BETA)
		c->err = (c->err * (c->exp-1.0) + fabs(p - c->pre)) / (double)c->exp;
	else
		c->err += BETA * (fabs(p - c->pre) - c->err);
	return c->err * c->num;
}
#endif

double cl_acc(CL *c)
{
	if(c->err <= EPS_0)
		return 1.0;
	else
		return ALPHA * pow(c->err / EPS_0, -NU);
}

void cl_update_fit(CL *c, double acc_sum, double acc)
{
	c->fit += BETA * ((acc * c->num) / acc_sum - c->fit);
}

double cl_update_size(CL *c, double num_sum)
{
	if(c->exp < 1.0/BETA)
		c->size = (c->size * (c->exp-1.0) + num_sum) / (double)c->exp; 
	else
		c->size += BETA * (num_sum - c->size);
	return c->size * c->num;
}

void cl_free(CL *c)
{
	cond_free(c);
	pred_free(c);
#ifdef SELF_ADAPT_MUTATION
	sam_free(c);
#endif
	free(c);
}

void cl_print(CL *c)
{
	cond_print(c);
	printf("%d %f %f %d %d %f %d\n",
			c->act, c->err, c->fit, c->num, c->exp, c->size, c->time);
	pred_print(c);
}
