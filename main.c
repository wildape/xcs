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
 * The main XCS module.  
 */

#include <stdio.h>
#include <stdlib.h>
#include "cons.h"
#include "random.h"
#include "cl.h"
#include "cl_set.h"
#include "env.h"
#include "perf.h"
#include "exp_single_step.h"
#include "exp_multi_step.h"

int main(int argc, char *argv[0])
{    
	if(argc < 3 || argc > 5) {
		printf("Usage: xcs problemType(mp,maze) problem{size|maze} [MaxTrials] [NumExp]\n");
		exit(EXIT_FAILURE);
	} 

	// initialise environment
	constants_init(argc, argv);
	random_init();
	env_init(argv);
	gen_outfname();

	// run experiments
	int perf[PERF_AVG_TRIALS];
	double err[PERF_AVG_TRIALS];
	for(int e = 1; e < NUM_EXPERIMENTS+1; e++) {
		printf("\nExperiment: %d\n", e);
		pop_init();
		outfile_init(e);
		if(!multi_step)
			single_step_exp(perf, err);
		else
			multi_step_exp(perf, err);
		// clean up
		set_kill(&pset);
		outfile_close();
	}
	return EXIT_SUCCESS;
}
