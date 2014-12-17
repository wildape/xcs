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
 * The performance output module.
 *
 * Writes system performance to a file and standard out.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "cons.h"
#include "perf.h"
#include "cl.h"
#include "cl_set.h"
  
FILE *fout;
char fname[30];
char basefname[30];

void gen_outfname()
{
	// file for writing output; uses the date/time/exp as file name
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(basefname, "dat/%04d-%02d-%02d-%02d%02d%02d", tm.tm_year + 1900, 
			tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void outfile_init(int exp_num)
{                	
	// create output file
	sprintf(fname, "%s-%d.dat", basefname, exp_num);
	fout = fopen(fname, "wt");
	if(fout == 0) {
		printf("Error opening file: %s. %s.\n", fname, strerror(errno));
		exit(EXIT_FAILURE);
	}       
}

void outfile_close()
{
	fclose(fout);
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
