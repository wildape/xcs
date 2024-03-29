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

double cl_update_err(CL *c, double p);
double cl_update_size(CL *c, double num_sum);

void cl_init(CL *c, int size, int time)
{
	cond_init(&c->cond);
	act_init(&c->act);
	pred_init(&c->pred);
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
	cond_copy(&to->cond, &from->cond);
	act_copy(&to->act, &from->act);
	pred_copy(&to->pred, &from->pred);
#ifdef SELF_ADAPT_MUTATION
	sam_copy(to, from);
#endif
}

void cl_cover(CL *c, char *state, int i)
{
	cond_cover(&c->cond, state);
	act_cover(&c->act, state, i);
}

_Bool cl_duplicate(CL *c1, CL *c2)
{
	if(cond_duplicate(&c1->cond, &c2->cond) 
			&& act_duplicate(&c1->act, &c2->act))
		return true;
	else
		return false;
}

_Bool cl_subsumes(CL *c1, CL *c2)
{
	if(act_duplicate(&c1->act, &c2->act) 
			&& c1->exp > THETA_SUB 
			&& c1->err < EPS_0
			&& cond_general(&c1->cond, &c2->cond))
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

void cl_update(CL *c, double *state, double p, int set_num)
{
	c->exp++;
	pred_compute(&c->pred, state);
	cl_update_err(c, p);
	pred_update(&c->pred, p, state);
	cl_update_size(c, set_num);
}

double cl_update_err(CL *c, double p)
{
	if(c->exp < 1.0/BETA) 
		c->err = (c->err * (c->exp-1.0) + fabs(p - c->pred.pre)) / (double)c->exp;
	else
		c->err += BETA * (fabs(p - c->pred.pre) - c->err);
	return c->err * c->num;
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
	cond_free(&c->cond);
	act_free(&c->act);
	pred_free(&c->pred);
#ifdef SELF_ADAPT_MUTATION
	sam_free(c);
#endif
	free(c);
}

void cl_print(CL *c)
{
	cond_print(&c->cond);
	act_print(&c->act);
	printf("%f %f %d %d %f %d\n", c->err, c->fit, c->num, c->exp, c->size, c->time);
	pred_print(&c->pred);
}
