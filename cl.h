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

#include "cond_ternary.h"
#include "act_integer.h"
#include "pred_nlms.h"
#include "pred_constant.h"

typedef struct CL
{
	COND cond;
	ACT act;
	PRED pred;
	double err;
	double fit;
	int num;
	int exp;
	double size;
	int time;
#ifdef SELF_ADAPT_MUTATION
	double *mu;
#endif
} CL;

// general classifier
_Bool cl_duplicate(CL *c1, CL *c2);
_Bool cl_subsumer(CL *c);
_Bool cl_subsumes(CL *c1, CL *c2);
double cl_acc(CL *c);
double cl_del_vote(CL *c, double avg_fit);
void cl_copy(CL *to, CL *from);
void cl_cover(CL *c, char *state, int i);
void cl_free(CL *c);
void cl_init(CL *c, int size, int time);
void cl_print(CL *c);
void cl_update(CL *c, double *state, double p, int set_num);
void cl_update_fit(CL *c, double acc_sum, double acc);

// classifier condition 
_Bool cond_crossover(COND *cond1, COND *cond2);
_Bool cond_duplicate(COND *cond1, COND *cond2);
_Bool cond_general(COND *cond1, COND *cond2);
_Bool cond_mutate(COND *cond, char *state);
_Bool cond_match(COND *cond, char *state);
void cond_copy(COND *to, COND *from);
void cond_free(COND *cond);
void cond_init(COND *cond);
void cond_cover(COND *cond, char *mcon);
void cond_print(COND *cond);
void cond_rand(COND *cond);

// classifier action
_Bool act_duplicate(ACT *act1, ACT *act2);
_Bool act_mutate(ACT *act);
void act_copy(ACT *to, ACT *from);
void act_free(ACT *act);
void act_init(ACT *act);
void act_cover(ACT *act, char *state, int i);
void act_print(ACT *act);
void act_rand(ACT *act);

// classifier prediction
double pred_compute(PRED *pred, double *state);
double pred_update_err(PRED *pred, double p, double *state);
void pred_update(PRED *pred, double p, double *state);
void pred_copy(PRED *to, PRED *from);
void pred_free(PRED *pred);
void pred_init(PRED *pred);
void pred_print(PRED *pred);

// self-adaptive mutation
void sam_adapt(CL *c);       
void sam_copy(CL *to, CL *from);
void sam_free(CL *c);
void sam_init(CL *c);
