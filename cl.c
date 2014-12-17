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
 * The classifier module.  
 *
 **************
 * Description: 
 **************
 * The classifier module.  
 *
 * Performs general operations applied to an individual classifier: creation,
 * copying, deletion, updating, and printing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "cons.h"
#include "random.h"
#include "cl.h"

void cl_init(CL *c, int size, int time)
{
	cond_init(c);
	act_init(c);
	pred_init(c);
	c->fit = INIT_FITNESS;
	c->err = INIT_ERROR;
	c->num = 1;
	c->exp = 0;
	c->size = size;
	c->time = time;
#ifdef SELF_ADAPT_MUTATION
	sam_init(c);
#endif
}

void cl_copy(CL *to, CL *from)
{
	cl_init(to, from->size, from->time);
	cond_copy(to, from);
	act_copy(to, from);
	pred_copy(to, from);
#ifdef SELF_ADAPT_MUTATION
	sam_copy(to, from);
#endif
}

_Bool mutate(CL *c, char *state)
{
#ifdef SELF_ADAPT_MUTATION
	sam_adapt(c);
	P_MUTATION = c->mu[0];
#endif
	_Bool mod = cond_mutate(c, state);
	if(act_mutate(c))
		mod = true;
	return mod;
}

_Bool cl_duplicate(CL *c1, CL *c2)
{
	if(cond_duplicate(c1,c2) && act_duplicate(c1,c2))
		return true;
	else
		return false;
}

_Bool cl_subsumes(CL *c1, CL *c2)
{
	if(act_duplicate(c1,c2) && c1->exp > THETA_SUB && c1->err < EPS_0)
		if(cond_general(c1,c2))
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

double cl_del_vote(CL *c, double avg_fit)
{
	if(c->fit / c->num >= DELTA * avg_fit || c->exp < THETA_DEL)
		return c->size * c->num;
	return c->size * c->num * avg_fit / (c->fit / c->num); 
}

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
	act_free(c);
	pred_free(c);
#ifdef SELF_ADAPT_MUTATION
	sam_free(c);
#endif
	free(c);
}

void cl_print(CL *c)
{
	cond_print(c);
	act_print(c);
	printf("%f %f %d %d %f %d\n", c->err, c->fit, c->num, c->exp, c->size, c->time);
	pred_print(c);
}
