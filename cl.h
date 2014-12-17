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
typedef struct CL
{
	char *con;
	int act;
	double err;
	double fit;
	int num;
	int exp;
	double size;
	int time;
	double pre;
#ifdef XCSF
	double *weights;
	int weights_length;
#endif
#ifdef SELF_ADAPT_MUTATION
	double *mu;
	int *iset;
	double *gset;
#endif
} CL;

// general classifier
_Bool cl_duplicate(CL *c1, CL *c2);
_Bool cl_subsumer(CL *c);
_Bool cl_subsumes(CL *c1, CL *c2);
_Bool mutate(CL *c, char *state);
double cl_acc(CL *c);
double cl_del_vote(CL *c, double avg_fit);
double cl_update_size(CL *c, double num_sum);
void cl_copy(CL *to, CL *from);
void cl_cover(CL *c, char *state, int i);
void cl_free(CL *c);
void cl_init(CL *c, int size, int time);
void cl_print(CL *c);
void cl_update_fit(CL *c, double acc_sum, double acc);

// classifier condition 
_Bool cond_crossover(CL *c1, CL *c2);
_Bool cond_duplicate(CL *c1, CL *c2);
_Bool cond_general(CL *c1, CL *c2);
_Bool cond_mutate(CL *c, char *state);
_Bool match(CL *c, char *state);
void cond_copy(CL *to, CL *from);
void cond_free(CL *c);
void cond_init(CL *c);
void cond_match(CL *c, char *mcon);
void cond_print(CL *c);
void cond_rand(CL *c);

// classifier action
_Bool act_duplicate(CL *c1, CL *c2);
_Bool act_mutate(CL *c);
void act_copy(CL *to, CL *from);
void act_free(CL *c);
void act_init(CL *c);
void act_match(CL *c, char *state, int i);
void act_print(CL *c);
void act_rand(CL *c);

// classifier prediction
#ifdef XCSF
double pred_compute(CL *c, double *state);
double pred_update_err(CL *c, double p, double *state);
void pred_update(CL *c, double p, double *state);
#else
double pred_update(CL *c, double p);
double pred_update_err(CL *c, double p);
#endif
void pred_copy(CL *to, CL *from);
void pred_free(CL *c);
void pred_init(CL *c);
void pred_print(CL *c);

// self-adaptive mutation
void sam_adapt(CL *c);       
void sam_copy(CL *to, CL *from);
void sam_free(CL *c);
void sam_init(CL *c);
