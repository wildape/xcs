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
_Bool mutate_con(CL *c, char *state);
#ifdef SELF_ADAPT_MUTATION
void adapt_mut(CL *c);
double gasdev(CL *c, int m);
#endif

void init_cl(CL *c, int size, int time)
{
	c->con = malloc(sizeof(char)*state_length);
	c->fit = INIT_FITNESS;
	c->err = INIT_ERROR;
	c->num = 1;
	c->exp = 0;
	c->size = size;
	c->time = time;
	pred_init(c);
#ifdef SELF_ADAPT_MUTATION
	c->mu = malloc(sizeof(double)*NUM_MU);
	c->iset = malloc(sizeof(int)*NUM_MU);
	c->gset = malloc(sizeof(double)*NUM_MU);
	for(int i = 0; i < NUM_MU; i++) {
		c->mu[i] = drand();
		c->iset[i] = 0;
		c->gset[i] = 0.0;
	}
#endif
}

void copy_cl(CL *to, CL *from)
{
	init_cl(to, from->size, from->time);
	memcpy(to->con, from->con, sizeof(char)*state_length);
	to->act = from->act;
	pred_copy(to, from);
#ifdef SELF_ADAPT_MUTATION
	memcpy(to->mu, from->mu, sizeof(double)*NUM_MU);
	memcpy(to->gset, from->gset, sizeof(double)*NUM_MU);
	memcpy(to->iset, from->iset, sizeof(int)*NUM_MU);
#endif
}

void rand_cond(CL *c)
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

void match_con(CL *c, char *state)
{
	for(int i = 0; i < state_length; i++) {
		if(drand() < P_DONTCARE)
			c->con[i] = DONT_CARE;
		else
			c->con[i] = state[i];
	}
}

void rand_act(CL *c)
{
	c->act = irand(0, num_actions);
}

_Bool match(CL *c, char *state)
{
	for(int i = 0; i < state_length; i++) {
		if(c->con[i] != DONT_CARE && c->con[i] != state[i])
			return false;
	}
	return true;
}

_Bool two_pt_cross(CL *c1, CL *c2) 
{
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

_Bool mutate(CL *c, char *state)
{
#ifdef SELF_ADAPT_MUTATION
	adapt_mut(c);
	P_MUTATION = c->mu[0];
#endif
	_Bool mod = mutate_con(c, state);
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

_Bool mutate_con(CL *c, char *state)
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

_Bool duplicate(CL *c1, CL *c2)
{
	if(strcmp(c1->con, c2->con) == 0 && c1->act == c2->act)
		return true;
	else
		return false;
}

_Bool subsumes(CL *c1, CL *c2)
{
	if(c1->act == c2->act && c1->exp > THETA_SUB && c1->err < EPS_0)
		if (general(c1, c2))
			return true;
	return false;
}

_Bool subsumer(CL *c)
{
	if(c->exp > THETA_SUB && c->err < EPS_0)
		return true;
	else
		return false;
}

_Bool general(CL *c1, CL *c2)
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

double del_vote(CL *c, double avg_fit)
{
	if(c->fit / c->num >= DELTA * avg_fit || c->exp < THETA_DEL)
		return c->size * c->num;
	return c->size * c->num * avg_fit / (c->fit / c->num); 
}

 
#ifdef XCSF
double update_err(CL *c, double p, double *state)
{
	double pre = pred_compute(c, state);
	if(c->exp < 1.0/BETA) 
		c->err = (c->err * (c->exp-1.0) + fabs(p - pre)) / (double)c->exp;
	else
		c->err += BETA * (fabs(p - pre) - c->err);
	return c->err * c->num;
}
 
#else
double update_err(CL *c, double p)
{
	if(c->exp < 1.0/BETA)
		c->err = (c->err * (c->exp-1.0) + fabs(p - c->pre)) / (double)c->exp;
	else
		c->err += BETA * (fabs(p - c->pre) - c->err);
	return c->err * c->num;
}
#endif

double acc(CL *c)
{
	if(c->err <= EPS_0)
		return 1.0;
	else
		return ALPHA * pow(c->err / EPS_0, -NU);
}

void update_fit(CL *c, double acc_sum, double acc)
{
	c->fit += BETA * ((acc * c->num) / acc_sum - c->fit);
}

double update_size(CL *c, double num_sum)
{
	if(c->exp < 1.0/BETA)
		c->size = (c->size * (c->exp-1.0) + num_sum) / (double)c->exp; 
	else
		c->size += BETA * (num_sum - c->size);
	return c->size * c->num;
}

void free_cl(CL *c)
{
	free(c->con);
	pred_free(c);
#ifdef SELF_ADAPT_MUTATION
	free(c->mu);
	free(c->iset);
	free(c->gset);
#endif
	free(c);
}

void print_cl(CL *c)
{
	for(int i = 0; i < state_length; i++)
		printf("%c", c->con[i]);
	printf("%d %f %f %d %d %f %d\n",
			c->act, c->err, c->fit, c->num, c->exp, c->size, c->time);
	pred_print(c);
}

#ifdef SELF_ADAPT_MUTATION
void adapt_mut(CL *c)
{
	for(int i = 0; i < NUM_MU; i++) {
		c->mu[i] *= exp(gasdev(c,i));
		if(c->mu[i] < muEPS_0)
			c->mu[i] = muEPS_0;
		else if(c->mu[i] > 1.0)
			c->mu[i] = 1.0;
	}
}

double gasdev(CL *c, int m)
{
	// from numerical recipes in c
	double fac, rsq, v1, v2;
	if(c->iset[m] == 0) {
		do {
			v1 = (drand()*2.0)-1.0;
			v2 = (drand()*2.0)-1.0;
			rsq = (v1*v1)+(v2*v2);
		}
		while(rsq >= 1.0 || rsq == 0.0);
		fac = sqrt(-2.0*log(rsq)/rsq);
		c->gset[m] = v1*fac;
		c->iset[m] = 1;
		return v2*fac;
	}
	else {
		c->iset[m] = 0;
		return c->gset[m];
	}
}
#endif
