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
 * The multi-step experiment module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "cons.h"
#include "cl.h"
#include "cl_set.h"
#include "pa.h"
#include "ga.h"
#include "env.h"
#include "perf.h"
#include "exp_multi_step.h"
 
int explore_multi(int step);
void exploit_multi(int *perf, double *err, int trial, int step);

void multi_step_exp(int *perf, double *err)
{
	pa_init();
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
	pa_free();
}

int explore_multi(int step)
{
	double prev_dstate[dstate_length];
	char prev_state[state_length];
	NODE *prev_aset = NULL, *kset = NULL;
	double prev_reward = 0.0;
	int steps, prev_asize = 0, prev_anum = 0;
	_Bool reset = false; 

	for(steps = 0; steps < TELETRANSPORTATION && !reset; steps++) {
		// percieve environment
		char *state = env_get_state();
		double *dstate = env_get_dstate();
		// generate match set
		NODE *mset = NULL;
		set_match(&mset, state, step+steps, &kset);
		// select a random move
		pa_build(&mset, dstate);
		int action = pa_rand_action();
		// generate action set
		NODE *aset = NULL; int anum = 0;
		int asize = set_action(&mset, &aset, action, &anum);
		// get environment feedback
		double reward = env_exec_action(action);
		reset = env_is_reset();
		// update previous action set and run GA
		if(prev_aset != NULL) {
			set_validate(&prev_aset, &prev_asize, &prev_anum);
			set_update(&prev_aset, &prev_asize, &prev_anum, 
					pa_best_val(), prev_reward, &kset, prev_dstate);
			ga(&prev_aset, prev_asize, prev_anum, step+steps, prev_state, &kset);
		}
		// in goal state, update current action set and run GA
		if(reset) {
			set_validate(&aset, &asize, &anum);
			set_update(&aset, &asize, &anum, 0.0, reward, &kset, dstate);
			ga(&aset, asize, anum, step+steps, state, &kset);
		}
		// next step
		set_free(&mset);
		set_free(&prev_aset);
		prev_aset = aset;
		prev_reward = reward;
		strncpy(prev_state, state, state_length);
		memcpy(prev_dstate, dstate, sizeof(double)*dstate_length);
	}
	set_free(&prev_aset);
	set_kill(&kset);
	return step+steps;
}

void exploit_multi(int *perf, double *err, int trial, int step)
{
	double prev_dstate[dstate_length];
	char prev_state[state_length];
	double prev_reward = 0.0, prev_pred = 0.0;
	int steps, prev_asize = 0, prev_anum = 0;
	NODE *prev_aset = NULL, *kset = NULL;
	err[trial%PERF_AVG_TRIALS] = 0.0;
	_Bool reset = false;

	for(steps = 0; steps < TELETRANSPORTATION && !reset; steps++) {
		// percieve environment
		char *state = env_get_state();
		double *dstate = env_get_dstate();
		// generate match set
		NODE *mset = NULL;
		set_match(&mset, state, step, &kset);
		// select the best move
		pa_build(&mset, dstate);
		int action = pa_best_action();
		// generate action set
		int anum = 0;
		NODE *aset = NULL;
		int asize = set_action(&mset, &aset, action, &anum);
		// get environment feedback
		double reward = env_exec_action(action);
		reset = env_is_reset();
		// update previous action set
		if(prev_aset != NULL) {
			set_validate(&prev_aset, &prev_asize, &prev_anum);
			set_update(&prev_aset, &prev_asize, &prev_anum, 
					pa_best_val(), prev_reward, &kset, prev_dstate);
			err[trial%PERF_AVG_TRIALS]+=fabs(GAMMA*pa_val(action)+prev_reward 
					-prev_pred)/max_payoff;
		}
		// in goal state, update current action set
		if(reset) {
			set_validate(&aset, &asize, &anum);
			set_update(&aset, &asize, &anum, 0.0, reward, &kset, dstate);
			err[trial%PERF_AVG_TRIALS]+=fabs(reward-pa_val(action))/max_payoff;
		}
		// next step
		set_free(&mset);
		set_free(&prev_aset);
		prev_aset = aset;
		prev_reward = reward;
		strncpy(prev_state, state, state_length);
		memcpy(prev_dstate, dstate, sizeof(double)*dstate_length);
		prev_pred = pa_val(action);
	}
	set_free(&prev_aset);
	set_kill(&kset);
	perf[trial%PERF_AVG_TRIALS] = steps;
	err[trial%PERF_AVG_TRIALS] /= steps;
}
