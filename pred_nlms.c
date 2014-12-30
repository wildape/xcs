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
 * The normalised least mean square classifier computed prediction module.
 *
 * Creates a weight vector representing a polynomial function to compute the
 * expected value given a problem instance and adapts the weights using the
 * least mean square update (also known as the modified Delta rule, or
 * Widrow-Hoff update.)
 */

#ifdef NLMS_PREDICTION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "random.h"
#include "cons.h"
#include "cl.h"

void pred_init(PRED *pred)
{
#ifdef QUADRATIC
	// offset(1) + n linear + n quadratic + n*(n-1)/2 mixed terms
	pred->weights_length = 1+2*dstate_length+dstate_length*(dstate_length-1)/2;
#else
	pred->weights_length = dstate_length+1;
#endif
	pred->weights = malloc(sizeof(double) * pred->weights_length);
	pred->weights[0] = XCSF_X0;
	for(int i = 1; i < pred->weights_length; i++)
		pred->weights[i] = 0.0;
}

void pred_copy(PRED *to, PRED *from)
{
	memcpy(to->weights, from->weights, from->weights_length);
}

void pred_free(PRED *pred)
{
	free(pred->weights);
}

void pred_update(PRED *pred, double p, double *state)
{
	// pre must have been updated for the current state previously in cl_update
	double error = p - pred->pre; //pred_compute(pred, state);
	double norm = XCSF_X0 * XCSF_X0;
	for(int i = 0; i < dstate_length; i++)
		norm += state[i] * state[i];
	double correction = (XCSF_ETA * error) / norm;
	// update first coefficient
	pred->weights[0] += XCSF_X0 * correction;
	int index = 1;
	// update linear coefficients
	for(int i = 0; i < dstate_length; i++)
		pred->weights[index++] += correction * state[i];
#ifdef QUADRATIC
	// update quadratic coefficients
	for(int i = 0; i < dstate_length; i++) {
		for(int j = i; j < dstate_length; j++) {
			pred->weights[index++] += correction * state[i] * state[j];
		}
	}
#endif
}

double pred_compute(PRED *pred, double *state)
{
	// first coefficient is offset
	double pre = XCSF_X0 * pred->weights[0];
	int index = 1;
	// multiply linear coefficients with the prediction input
	for(int i = 0; i < dstate_length; i++)
		pre += pred->weights[index++] * state[i];
#ifdef QUADRATIC
	// multiply quadratic coefficients with prediction input
	for(int i = 0; i < dstate_length; i++) {
		for(int j = i; j < dstate_length; j++) {
			pre += pred->weights[index++] * state[i] * state[j];
		}
	}
#endif
	pred->pre = pre;
	return pre;
} 

void pred_print(PRED *pred)
{
	printf("weights: ");
	for(int i = 0; i < pred->weights_length; i++)
		printf("%f, ", pred->weights[i]);
	printf("\n");
}

#endif
