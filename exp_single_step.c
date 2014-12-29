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
 * The single-step experiment module.
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
#include "exp_single_step.h"
 
void explore_single(int time);
void exploit_single(int time, int *correct, double *error);

void single_step_exp(int *perf, double *err)
{
	pa_init();
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
	pa_free();
}
 
void explore_single(int time)
{
	char *state = env_get_state();
	NODE *mset = NULL, *kset = NULL;
	set_match(&mset, state, time, &kset);
	double *dstate = env_get_dstate();
	pa_build(&mset, dstate);
	int action = pa_rand_action();
	NODE *aset = NULL; int anum = 0;
	int asize = set_action(&mset, &aset, action, &anum);
	double reward = env_exec_action(action);
	set_update(&aset, &asize, &anum, 0.0, reward, &kset, dstate);
	ga(&aset, asize, anum, time, state, &kset);
	set_free(&aset);
	set_free(&mset);
	set_kill(&kset);
}

void exploit_single(int time, int *correct, double *error)
{
	char *state = env_get_state();
	NODE *mset = NULL, *kset = NULL;
	set_match(&mset, state, time, &kset);
	double *dstate = env_get_dstate();
	pa_build(&mset, dstate);
	int action = pa_best_action();
	NODE *aset = NULL; int anum = 0;
	set_action(&mset, &aset, action, &anum);
	double reward = env_exec_action(action);
	if(reward > 0)
		correct[time%PERF_AVG_TRIALS] = 1;
	else
		correct[time%PERF_AVG_TRIALS] = 0;
	error[time%PERF_AVG_TRIALS] = fabs(reward - pa_best_val());
	set_free(&aset);
	set_free(&mset);
	set_kill(&kset);
}
 
