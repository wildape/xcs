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
#include <errno.h>
#include <time.h>
#include "cons.h"
#include "random.h"
#include "cl.h"
#include "cl_set.h"
#include "ga.h"
#include "env.h"

void single_step_exp(int *perf, double *err);
void explore_single(int time);
void exploit_single(int time, int *correct, double *error);
void multi_step_exp(int *perf, double *err);
int explore_multi(int step);
void exploit_multi(int *perf, double *err, int trial, int step);
void disp_perf(int *performance, double *error, int expl_p);

FILE *fout;

int main(int argc, char *argv[0])
{    
	if(argc < 3 || argc > 5) {
		printf("Usage: xcs problemType(mp,maze) problem{size|maze} [MaxTrials] [NumExp]\n");
		exit(EXIT_FAILURE);
	} 
	// file for writing output; uses the date/time/exp as file name
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char basefname[30];
	char fname[30];
	sprintf(basefname, "dat/%04d-%02d-%02d-%02d%02d%02d", tm.tm_year + 1900, 
			tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  	
	// initialise constants
	init_constants();
	if(argc > 3) {
		MAX_TRIALS = atoi(argv[3]);
		if(argc > 4)
			NUM_EXPERIMENTS = atoi(argv[4]);
	}    
	int perf[PERF_AVG_TRIALS];
	double err[PERF_AVG_TRIALS];
	// initialise environment
	init_random();
	init_env(argv);
	// start experiments
	for(int i = 1; i < NUM_EXPERIMENTS+1; i++) {
		// new output file
		sprintf(fname, "%s-%d.dat", basefname, i);
		fout = fopen(fname, "wt");
		if(fout == 0) {
			printf("Error opening file: %s. %s.\n", fname, strerror(errno));
			exit(EXIT_FAILURE);
		} 

		printf("\nExperiment: %d\n", i);
		pset = NULL; // population linked list
		pop_num = 0; // num macro-classifiers
		pop_num_sum = 0; // numerosity sum
		if(!multi_step)
			single_step_exp(perf, err);
		else
			multi_step_exp(perf, err);
		set_kill(&pset);
		fclose(fout);
	}
	return EXIT_SUCCESS;
}

void single_step_exp(int *perf, double *err)
{
	int expl = 0;
	for(int expl_p = 0; expl_p < MAX_TRIALS; expl_p += expl) {
		expl = (expl+1)%2;
		if(expl == 1)
			explore_single(expl_p);
		else
			exploit_single(expl_p, perf, err);
		if(expl_p%PERF_AVG_TRIALS == 0 && expl == 0 && expl_p > 0)
			disp_perf(perf, err, expl_p);
	}
}
 
void explore_single(int time)
{
	char *state = get_state();
	NODE *mset = NULL, *kset = NULL;
	set_match(&mset, state, time, &kset);
#ifdef XCSF
	double *dstate = get_dstate();
	pa_init(&mset, dstate);
#else
	pa_init(&mset);
#endif
	int action = pa_rand_action();
	NODE *aset = NULL; int anum = 0;
	int asize = set_action(&mset, &aset, action, &anum);
	double reward = execute_action(action);
#ifdef XCSF
	set_update(&aset, &asize, &anum, 0.0, reward, &kset, dstate);
#else
	set_update(&aset, &asize, &anum, 0.0, reward, &kset);
#endif
	ga(&aset, asize, anum, time, state, &kset);
	set_free(&aset);
	set_clean(&kset, &mset, true);
	set_free(&mset);
}

void exploit_single(int time, int *correct, double *error)
{
	char *state = get_state();
	NODE *mset = NULL, *kset = NULL;
	set_match(&mset, state, time, &kset);
#ifdef XCSF
	double *dstate = get_dstate();
	pa_init(&mset, dstate);
#else
	pa_init(&mset);
#endif
	int action = pa_best_action();
	NODE *aset = NULL; int anum = 0;
	set_action(&mset, &aset, action, &anum);
	double reward = execute_action(action);
	if(reward > 0)
		correct[time%PERF_AVG_TRIALS] = 1;
	else
		correct[time%PERF_AVG_TRIALS] = 0;
	error[time%PERF_AVG_TRIALS] = fabs(reward - pa_best_val());
	set_free(&aset);
	set_clean(&kset, &mset, true);
	set_free(&mset);
}
 
void multi_step_exp(int *perf, double *err)
{
	int expl = 0;
	int expl_step = 0;
	for(int expl_trial = 0; expl_trial < MAX_TRIALS; expl_trial += expl) {
		expl = (expl+1)%2;
		env_reset();
		if(expl == 1)
			expl_step = explore_multi(expl_step);
		else
			exploit_multi(perf, err, expl_trial, expl_step);
		if(expl_trial%PERF_AVG_TRIALS == 0 && expl == 0 && expl_trial > 0)
			disp_perf(perf, err, expl_trial);
	}
}

int explore_multi(int step)
{
#ifdef XCSF
	double prev_dstate[pred_length];
#endif
	char prev_state[state_length];
	NODE *prev_aset = NULL, *kset = NULL;
	double prev_reward = 0.0;
	int steps, prev_asize = 0, prev_anum = 0;
	_Bool reset = false; 

	for(steps = 0; steps < TELETRANSPORTATION && !reset; steps++) {
		// percieve environment
		char *state = get_state();
#ifdef XCSF
		double *dstate = get_dstate();
#endif
		// generate match set
		NODE *mset = NULL;
		set_match(&mset, state, step+steps, &kset);
		// select a random move
#ifdef XCSF
		pa_init(&mset, dstate);
#else
		pa_init(&mset);
#endif
		int action = pa_rand_action();
		// generate action set
		NODE *aset = NULL; int anum = 0;
		int asize = set_action(&mset, &aset, action, &anum);
		// get environment feedback
		double reward = execute_action(action);
		reset = is_reset();
		// update previous action set and run GA
		if(prev_aset != NULL) {
			set_validate(&prev_aset, &prev_asize, &prev_anum);
#ifdef XCSF
			set_update(&prev_aset, &prev_asize, &prev_anum, 
					pa_best_val(), prev_reward, &kset, prev_dstate);
#else
			set_update(&prev_aset, &prev_asize, &prev_anum, 
					pa_best_val(), prev_reward, &kset);
#endif
			ga(&prev_aset, prev_asize, prev_anum, step+steps, prev_state, &kset);
		}
		// in goal state, update current action set and run GA
		if(reset) {
			set_validate(&aset, &asize, &anum);
#ifdef XCSF
			set_update(&aset, &asize, &anum, 0.0, reward, &kset, dstate);
#else
			set_update(&aset, &asize, &anum, 0.0, reward, &kset);
#endif
			ga(&aset, asize, anum, step+steps, state, &kset);
		}
		// next step
		set_free(&mset);
		set_free(&prev_aset);
		prev_aset = aset;
		set_clean(&kset, &prev_aset, false);
		prev_reward = reward;
		strncpy(prev_state, state, state_length);
#ifdef XCSF
		memcpy(prev_dstate, dstate, sizeof(double)*pred_length);
#endif
	}
	set_clean(&kset, &prev_aset, true);
	set_free(&prev_aset);
	return step+steps;
}

void exploit_multi(int *perf, double *err, int trial, int step)
{
#ifdef XCSF
	double prev_dstate[pred_length];
	char prev_state[state_length];
#endif
	double prev_reward = 0.0, prev_pred = 0.0;
	int steps, prev_asize = 0, prev_anum = 0;
	NODE *prev_aset = NULL, *kset = NULL;
	err[trial%PERF_AVG_TRIALS] = 0.0;
	_Bool reset = false;

	for(steps = 0; steps < TELETRANSPORTATION && !reset; steps++) {
		// percieve environment
		char *state = get_state();
#ifdef XCSF
		double *dstate = get_dstate();
#endif
		// generate match set
		NODE *mset = NULL;
		set_match(&mset, state, step, &kset);
		// select the best move
#ifdef XCSF
		pa_init(&mset, dstate);
#else
		pa_init(&mset);
#endif
		int action = pa_best_action();
		// generate action set
		int anum = 0;
		NODE *aset = NULL;
		int asize = set_action(&mset, &aset, action, &anum);
		// get environment feedback
		double reward = execute_action(action);
		reset = is_reset();
		// update previous action set
		if(prev_aset != NULL) {
			set_validate(&prev_aset, &prev_asize, &prev_anum);
#ifdef XCSF
			set_update(&prev_aset, &prev_asize, &prev_anum, 
					pa_best_val(), prev_reward, &kset, prev_dstate);
#else
			set_update(&prev_aset, &prev_asize, &prev_anum, 
					pa_best_val(), prev_reward, &kset);
#endif
			err[trial%PERF_AVG_TRIALS]+=fabs(GAMMA*pa_val(action)+prev_reward 
					-prev_pred)/max_payoff;
		}
		// in goal state, update current action set
		if(reset) {
			set_validate(&aset, &asize, &anum);
#ifdef XCSF
			set_update(&aset, &asize, &anum, 0.0, reward, &kset, dstate);
#else
			set_update(&aset, &asize, &anum, 0.0, reward, &kset);
#endif
			err[trial%PERF_AVG_TRIALS]+=fabs(reward-pa_val(action))/max_payoff;
		}
		// next step
		set_free(&mset);
		set_free(&prev_aset);
		prev_aset = aset;
		set_clean(&kset, &prev_aset, false);
		prev_reward = reward;
#ifdef XCSF
		strncpy(prev_state, state, state_length);
		memcpy(prev_dstate, dstate, sizeof(double)*pred_length);
#endif
		prev_pred = pa_val(action);
	}
	set_clean(&kset, &prev_aset, true);
	set_free(&prev_aset);
	perf[trial%PERF_AVG_TRIALS] = steps;
	err[trial%PERF_AVG_TRIALS] /= steps;
}

void disp_perf(int *performance, double *error, int expl_p)
{
	double perf = 0.0;
	double serr = 0.0;
	for(int i = 0; i < PERF_AVG_TRIALS; i++) {
		perf += performance[i];
		serr += error[i];
	}
	perf /= (double)PERF_AVG_TRIALS;
	serr /= (double)PERF_AVG_TRIALS;
	printf("%d %.2f %.5f %d", expl_p, perf, serr, pop_num);
	fprintf(fout, "%d %.2f %.5f %d", expl_p, perf, serr, pop_num);
#ifdef SELF_ADAPT_MUTATION
	for(int i = 0; i < NUM_MU; i++) {
		printf(" %.5f", set_avg_mut(&pset, i));
		fprintf(fout, " %.5f", set_avg_mut(&pset, i));
	}
#endif
	printf("\n");
	fprintf(fout, "\n");
	fflush(stdout);
	fflush(fout);
}
