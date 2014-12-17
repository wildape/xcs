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
#include "cons.h"
#include "random.h"
#include "cl.h"
#include "cl_set.h"

NODE *pop_del();
CL *select_rw(NODE **set, double fit_sum);
_Bool action_covered(NODE **set, int action);
double set_total_fit(NODE **set);
double mean_time(NODE **set, int num_sum);
double total_time(NODE **set);
void pop_add_cl(CL *c);
void set_add_cl(NODE **set, CL *c);
void ga_subsume(CL *c, CL *c1p, CL *c2p, NODE **set, int size);
void set_subsumption(NODE **set, int *size, int *num, NODE **kset);
void set_times(NODE **set, int time);
void update_set_fit(NODE **set, int size, int num_sum);

double *pa;
double *nr;

void match_set(NODE **mset, char *state, int time, NODE **kset)
{
	int m_num = 0;
	int m_size = 0;
	_Bool act_covered[num_actions];
	for(int i = 0; i< num_actions; i++)
		act_covered[i] = false;
	// find matching classifiers in the population
	for(NODE *iter = pset; iter != NULL; iter = iter->next) {
		if(match(iter->cl, state)) {
			set_add_cl(mset, iter->cl);
			act_covered[iter->cl->act] = true;
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
				init_cl(new, m_num+1, time);
				match_con(new, state);
				new->act = i;
				pop_add_cl(new);
				set_add_cl(mset, new);
				act_covered[i]++; 
			}
		}
		// enforce population size limit
		while(pop_num_sum > POP_SIZE) {
			NODE * del = pop_del();
			if(match(del->cl, state)) {
				set_validate(mset, &m_size, &m_num);
				if(!action_covered(mset, del->cl->act)) { 
					act_covered[del->cl->act] = false;
					again = true;
				}     
			}
			if(del->cl->num == 0) {
				set_add_cl(kset, del->cl);
			    free(del);
			}
		}
	} while(again);
}

_Bool action_covered(NODE **set, int action)
{
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		if(iter->cl->act == action)
			return true;
	}
	return false;
}

int action_set(NODE **mset, NODE **aset, int action, int *num)
{
	int size = 0;
	for(NODE *iter = *mset; iter != NULL; iter = iter->next) {
		if(iter->cl->act == action) {
			size++;
			*num += iter->cl->num;
			set_add_cl(aset, iter->cl);
		}
	}   
	return size;
}

void set_add_cl(NODE **set, CL *c)
{
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

void pop_add_cl(CL *c)
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
			if(duplicate(c, iter->cl)) {
				iter->cl->num++;
				free_cl(c);
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

NODE *pop_del()
{
	NODE *iter;
	NODE *prev = NULL;
	double avg_fit = set_total_fit(&pset) / pop_num_sum;
	double sum = 0.0;
	for(iter = pset; iter != NULL; iter = iter->next)
		sum += del_vote(iter->cl, avg_fit);
	double p = drand() * sum;
	sum = 0.0;
	for(iter=pset; iter != NULL; prev=iter, iter=iter->next) {
		sum += del_vote(iter->cl, avg_fit);
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
			}
			return iter;
		}
	}   
	return iter;
}

void ga(NODE **set, int size, int num_sum, int time, char *state, NODE **kset)
{
	// check if ga should be run
	if(size == 0 || time - mean_time(set, num_sum) < THETA_GA)
		return;
	set_times(set, time);
	// select parents
	double fit_sum = set_total_fit(set);
	CL *c1p = select_rw(set, fit_sum);
	CL *c2p = select_rw(set, fit_sum);
	// create copies of parents
	CL *c1 = malloc(sizeof(CL));
	CL *c2 = malloc(sizeof(CL));
	copy_cl(c1, c1p);
	copy_cl(c2, c2p);
	// apply genetic operators to offspring
	two_pt_cross(c1, c2);
	mutate(c1, state);
	mutate(c2, state);
	// reduce offspring pre, err, fit
#ifndef XCSF
	c1->pre = (c1p->pre + c2p->pre)/2.0;
	c2->pre = c1->pre;
#endif
	c1->err = ERR_REDUC * ((c1p->err + c2p->err)/2.0);
	c2->err = c1->err;
	c1->fit = c1p->fit / c1p->num;
	c2->fit = c2p->fit / c2p->num;
	c1->fit = FIT_REDUC * (c1->fit + c2->fit)/2.0;
	c2->fit = c1->fit;
	// add offspring to population
	if(GA_SUBSUMPTION) {
		ga_subsume(c1, c1p, c2p, set, size);
		ga_subsume(c2, c1p, c2p, set, size);
	}
	else {
		pop_add_cl(c1);
		pop_add_cl(c2);
	}
	// enforce population size limit
	while(pop_num_sum > POP_SIZE) {
		NODE *del = pop_del();
		if(del->cl->num == 0) {
            set_add_cl(kset, del->cl);
			free(del);
		}
	}
}   

void ga_subsume(CL *c, CL *c1p, CL *c2p, NODE **set, int size)
{
	// check if either parent subsumes the offspring
	if(subsumes(c1p, c)) {
		c1p->num++;
		pop_num_sum++;
		free_cl(c);
	}
	else if(subsumes(c2p, c)) {
		c2p->num++;
		pop_num_sum++;
		free_cl(c);
	}
	// attempt to find a random subsumer from the action set
	else {
		NODE *candidates[size];
		int choices = 0;
		for(NODE *iter = *set; iter != NULL; iter = iter->next) {
			if(subsumes(iter->cl, c)) {
				candidates[choices] = iter;
				choices++;
			}
		}
		// found
		if(choices > 0) {
			candidates[irand(0,choices)]->cl->num++;
			pop_num_sum++;
			free_cl(c);
		}
		// no subsumers found, add offspring to population
		else {
			pop_add_cl(c);   
		}
	}
}

#ifdef XCSF
void update_set(NODE **set, int *size, int *num, 
		double max_p, double r, NODE **kset, double *state)
#else
void update_set(NODE **set, int *size, int *num, 
		double max_p, double r, NODE **kset)
#endif
{
	double p = r + (GAMMA * max_p);
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		CL *c = iter->cl;
		c->exp++;
#ifdef XCSF
		update_err(c, p, state);
		pred_update(c, p, state);
#else
		update_err(c, p);
		pred_update(c, p);
#endif
		update_size(c, *num);
	}
	update_set_fit(set, *size, *num);
	if(ACTION_SUBSUMPTION)
		set_subsumption(set, size, num, kset);
}

void update_set_fit(NODE **set, int size, int num_sum)
{
	double acc_sum = 0.0;
	double accs[size];
	// calculate accuracies
	int i = 0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		accs[i] = acc(iter->cl);
		acc_sum += accs[i] * num_sum;
		i++;
	}
	// update fitnesses
	i = 0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next) {
		update_fit(iter->cl, acc_sum, accs[i]);
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
		if(subsumer(c)) {
			if(s == NULL || general(c, s))
				s = c;
		}
	}
	// subsume the more specific classifiers in the set
	if(s != NULL) {
		iter = *set; 
		while(iter != NULL) {
			CL *c = iter->cl;
			if(general(s, c)) {
				s->num += c->num;
				c->num = 0;
				set_add_cl(kset, c);
				set_validate(set, size, num);
				set_validate(&pset, &pop_num, &pop_num_sum);
			}
			iter = iter->next;
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
			if(prev == NULL)
				*set = iter->next;
			else
				prev->next = iter->next;
			iter = iter->next;
			continue;
		}
		else {
			(*size)++;
			(*num) += iter->cl->num;
		}
		prev = iter;
		iter = iter->next;
	}
}

void print_set(NODE *set)
{
	for(NODE *iter = set; iter != NULL; iter = iter->next)
		print_cl(iter->cl);
}

#ifdef XCSF
void init_pa(NODE **set, double *state)
#else
void init_pa(NODE **set)
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

CL *select_rw(NODE **set, double fit_sum)
{
	double p = drand() * fit_sum;
	NODE *iter = *set;
	double sum = iter->cl->fit;
	while(p > sum) {
		iter = iter->next;
		sum += iter->cl->fit;
	}
	return iter->cl;
}

double total_time(NODE **set)
{
	double sum = 0.0;
	for(NODE *iter = *set; iter != NULL; iter = iter->next)
		sum += iter->cl->time * iter->cl->num;
	return sum;
}

double mean_time(NODE **set, int num_sum)
{
	return total_time(set) / num_sum;
}

void free_set(NODE **set)
{
	// frees the set only, not the classifiers
	NODE *iter = *set;
	while(iter != NULL) {
		*set = iter->next;
		free(iter);
		iter = *set;
	}
}

void kill_set(NODE **set)
{
	// frees the set and classifiers
	NODE *iter = *set;
	while(iter != NULL) {
		free_cl(iter->cl);
		*set = iter->next;
		free(iter);
		iter = *set;
	}
}

void clean_set(NODE **kset, NODE **set, _Bool in_set)
{
	// if in_set = false, removes classifiers from kset
	// that are *not* in the set; otherwise removes only
	// classifiers from kset that *are* in the set
	NODE *iter, *kiter, *prev_kiter = NULL;
	for(iter = *set; iter != NULL; iter = iter->next) {
		for(kiter = *kset; kiter != NULL; kiter = kiter->next) {
			if((!in_set && iter->cl != kiter->cl) ||
					(in_set && iter->cl == kiter->cl)) {
				if(prev_kiter == NULL)
					*kset = kiter->next;
				else
					prev_kiter->next = kiter->next;
				free_cl(kiter->cl);
				free(kiter);
			}
		}
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
