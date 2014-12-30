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
 * The classifier set module.
 *
 * Performs operations applied to sets of classifiers: creation, deletion,
 * updating, prediction, validation, printing.  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cons.h"
#include "random.h"
#include "cl.h"
#include "cl_set.h"

_Bool set_action_covered(NODE **set, int action);
void set_subsumption(NODE **set, int *size, int *num, NODE **kset);
void set_update_fit(NODE **set, int size, int num_sum);

void pop_init()
{
	pset = NULL; // population linked list
	pop_num = 0; // num macro-classifiers
	pop_num_sum = 0; // numerosity sum

	if(POP_INIT) {
		while(pop_num < POP_SIZE) {
			CL *new = malloc(sizeof(CL));
			cl_init(new, POP_SIZE, 0);
			cond_rand(&new->cond);
			act_rand(&new->act);
			pop_add(new);
		}
	}
}

void set_match(NODE **mset, char *state, int time, NODE **kset)
{
	// builds the match set
	int m_num = 0;
	int m_size = 0;
	_Bool act_covered[num_actions];
	for(int i = 0; i < num_actions; i++)
		act_covered[i] = false;

	// find matching classifiers in the population
	for(NODE *iter = pset; iter != NULL; iter = iter->next) {
		if(cond_match(&iter->cl->cond, state)) {
			set_add(mset, iter->cl);
			act_covered[iter->cl->act.a] = true;
			m_num += iter->cl->num;
			m_size++;
		}
	}   

	// perform covering if all actions are not represented
	_Bool again;
	do {
		again = false;
		for(int i = 0; i < num_actions; i++) {
			if(!act_covered[i]) {
				// new classifier with matching condition & action
				CL *new = malloc(sizeof(CL));
				cl_init(new, m_num+1, time);
				cl_cover(new, state, i);
				pop_add(new);
				set_add(mset, new);
				m_size++;
				m_num++;
				act_covered[i] = true;
			}
		}

		// enforce pop size
		int prev_psize = pop_num;
		pop_enforce_limit(kset);
		// if a macro classifier was deleted, validate the match set
		if(prev_psize > pop_num) {
			int prev_msize = m_size;
			set_validate(mset, &m_size, &m_num);
			// if the deleted classifier was in the match set,
			// check if an action is now not covered
			if(prev_msize > m_size) {
				for(int i = 0; i < num_actions; i++) {
					if(!set_action_covered(mset, i)) {
						act_covered[i] = false;
						again = true;
					}
				}
			}
		}
	} while(again);
}

_Bool set_action_covered(NODE **set, int action)
{
	// check whether an action is represented in the set
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		if(iter->cl->act.a == action)
			return true;
	}
	return false;
}

int set_action(NODE **mset, NODE **aset, int action, int *num)
{
	// builds the action set
	int size = 0;
	for(NODE *iter = *mset; iter != NULL; iter = iter->next) {
		if(iter->cl->act.a == action) {
			size++;
			*num += iter->cl->num;
			set_add(aset, iter->cl);
		}
	}   
	return size;
}

void set_add(NODE **set, CL *c)
{
	// add a classifier to a set
	if(*set == NULL) {
		*set = malloc(sizeof(NODE));
		(*set)->cl = c;
		(*set)->next = NULL;
	}
	else {
		NODE *new = malloc(sizeof(NODE));
		new->cl = c;
		new->next = *set;
		*set = new;
	}
}

void pop_add(CL *c)
{
	pop_num_sum++;
	// adds the initial node in the list
	if(pset == NULL) {
		pset = malloc(sizeof(NODE));
		pset->cl = c;
		pset->next = NULL;
		pop_num++;
	}
	// adds a new node at the start of the list
	else {
		// if a duplicate exists just increase numerosity
		for(NODE *iter = pset; iter != NULL; iter = iter->next) {
			if(cl_duplicate(c, iter->cl)) {
				iter->cl->num++;
				cl_free(c);
				return;
			}
		}   
		// new classifier
		NODE *new = malloc(sizeof(NODE));
		new->next = pset;
		new->cl = c;
		pset = new;
		pop_num++;
	}
}

void pop_del(NODE **kset)
{
	double avg_fit = set_total_fit(&pset) / pop_num_sum; double sum = 0.0;
	for(NODE *iter = pset; iter != NULL; iter = iter->next) sum +=
		cl_del_vote(iter->cl, avg_fit); double p = drand() * sum;

	NODE *prev = NULL;
	sum = 0.0;
	for(NODE *iter = pset; iter != NULL; iter = iter->next) {
		sum += cl_del_vote(iter->cl, avg_fit);
		if(sum > p) {
			iter->cl->num--;
			pop_num_sum--;
			// macro classifier must be deleted
			if(iter->cl->num == 0) {
				pop_num--;
				if(prev == NULL)
					pset = iter->next;
				else
					prev->next = iter->next;
				set_add(kset, iter->cl);
				free(iter);
			}
			return;
		}
		prev = iter;
	}   
}

void pop_enforce_limit(NODE **kset)
{
	while(pop_num_sum > POP_SIZE)
		pop_del(kset);
}

void set_update(NODE **set, int *size, int *num, 
		double max_p, double r, NODE **kset, double *state)
{
	double p = r + (GAMMA * max_p);

	for(NODE *iter = *set; iter != NULL; iter = iter->next)
		cl_update(iter->cl, state, p, *num);
	set_update_fit(set, *size, *num);

	if(ACTION_SUBSUMPTION)
		set_subsumption(set, size, num, kset);
}

void set_update_fit(NODE **set, int size, int num_sum)
{
	double acc_sum = 0.0;
	double accs[size];
	// calculate accuracies
	int i = 0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		accs[i] = cl_acc(iter->cl);
		acc_sum += accs[i] * num_sum;
		i++;
	}
	// update fitnesses
	i = 0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		cl_update_fit(iter->cl, acc_sum, accs[i]);
		i++;
	}
}

void set_subsumption(NODE **set, int *size, int *num, NODE **kset)
{
	CL *s = NULL;
	NODE *iter;
	// find the most general subsumer in the set
	for(iter = *set; iter != NULL; iter = iter->next) {
		CL *c = iter->cl;
		if(cl_subsumer(c)) {
			if(s == NULL || cond_general(&c->cond, &s->cond))
				s = c;
		}
	}
	// subsume the more specific classifiers in the set
	if(s != NULL) {
		iter = *set; 
		while(iter != NULL) {
			CL *c = iter->cl;
			iter = iter->next;
			if(cond_general(&s->cond, &c->cond)) {
				s->num += c->num;
				c->num = 0;
				set_add(kset, c);
				set_validate(set, size, num);
				set_validate(&pset, &pop_num, &pop_num_sum);
			}
		}
	}
}

void set_validate(NODE **set, int *size, int *num)
{
	// remove nodes pointing to classifiers with 0 numerosity
	*size = 0;
	*num = 0;
	NODE *prev = NULL;
	NODE *iter = *set;
	while(iter != NULL) {
		if(iter->cl == NULL || iter->cl->num == 0) {
			if(prev == NULL) {
				*set = iter->next;
				free(iter);
				iter = *set;
			}
			else {
				prev->next = iter->next;
				free(iter);
				iter = prev->next;
			}
		}
		else {
			(*size)++;
			(*num) += iter->cl->num;
			prev = iter;
			iter = iter->next;
		}
	}
}

void set_print(NODE *set)
{
	for(NODE *iter = set; iter != NULL; iter = iter->next)
		cl_print(iter->cl);
}

void set_times(NODE **set, int time)
{
	for(NODE *iter = *set; iter != NULL; iter = iter->next)
		iter->cl->time = time;
}

double set_total_fit(NODE **set)
{
	double sum = 0.0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next)
		sum += iter->cl->fit;
	return sum;
}

double set_total_time(NODE **set)
{
	double sum = 0.0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next)
		sum += iter->cl->time * iter->cl->num;
	return sum;
}

double set_mean_time(NODE **set, int num_sum)
{
	return set_total_time(set) / num_sum;
}

void set_free(NODE **set)
{
	// frees the set only, not the classifiers
	NODE *iter = *set;
	while(iter != NULL) {
		*set = iter->next;
		free(iter);
		iter = *set;
	}
}

void set_kill(NODE **set)
{
	// frees the set and classifiers
	NODE *iter = *set;
	while(iter != NULL) {
		cl_free(iter->cl);
		*set = iter->next;
		free(iter);
		iter = *set;
	}
}

#ifdef SELF_ADAPT_MUTATION
double set_avg_mut(NODE **set, int m)
{
	double sum = 0.0;
	int cnt = 0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		sum += iter->cl->mu[m];
		cnt++;
	}
	return sum/cnt;
}
#endif
