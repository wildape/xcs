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
typedef struct NODE
{
	CL *cl;
	struct NODE *next;
} NODE;
 
int pa_best_action();
int pa_rand_action();
double pa_best_val();
double pa_val(int act);
int action_set(NODE **mset, NODE **aset, int action, int *num);
void match_set(NODE **mset, char *state, int time, NODE **kset);
void ga(NODE **set, int size, int num_sum, int time, char *state, NODE **kset);
void set_validate(NODE **set, int *size, int *num);
void print_set(NODE *set);
void free_set(NODE **set);
void kill_set(NODE **set);
void clean_set(NODE **kset, NODE **set, _Bool in_set);
#ifdef XCSF
void init_pa(NODE **set, double *state);
void update_set(NODE **set, int *size, int *num, double max_p, double r,
		NODE **kset, double *state);
#else
void update_set(NODE **set, int *size, int *num, double max_p, double r, 
		NODE **kset);
void init_pa(NODE **set);
#endif
#ifdef SELF_ADAPT_MUTATION
double set_avg_mut(NODE **set, int m);
#endif

NODE *pset;
int pop_num;
int pop_num_sum;
