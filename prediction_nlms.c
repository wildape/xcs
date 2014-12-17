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

#ifdef XCSF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "random.h"
#include "cons.h"
#include "cl.h"

void pred_init(CL *c)
{
#ifdef QUADRATIC
	// offset(1) + n linear + n quadratic + n*(n-1)/2 mixed terms
	c->weights_length = 1+2*pred_length+pred_length*(pred_length-1)/2;
#else
	c->weights_length = pred_length+1;
#endif
	c->weights = malloc(sizeof(double) * c->weights_length);
	c->weights[0] = XCSF_X0;
	for(int i = 1; i < c->weights_length; i++)
		c->weights[i] = 0.0;
}

void pred_copy(CL *to, CL *from)
{
	memcpy(to->weights, from->weights, sizeof(double)*from->weights_length);
}

void pred_free(CL *c)
{
	free(c->weights);
}
 
double pred_update_err(CL *c, double p)
{
	// pre has been updated for the current state during pa_init()
	double pre = c->pre; //pred_compute(c, state);
	if(c->exp < 1.0/BETA) 
		c->err = (c->err * (c->exp-1.0) + fabs(p - pre)) / (double)c->exp;
	else
		c->err += BETA * (fabs(p - pre) - c->err);
	return c->err * c->num;
}
 
void pred_update(CL *c, double p, double *state)
{
	// pre has been updated for the current state during pa_init()
	double error = p - c->pre; //pred_compute(c, state);
	double norm = XCSF_X0 * XCSF_X0;
	for(int i = 0; i < pred_length; i++)
		norm += state[i] * state[i];
	double correction = (XCSF_ETA * error) / norm;
	// update first coefficient
	c->weights[0] += XCSF_X0 * correction;
	int index = 1;
	// update linear coefficients
	for(int i = 0; i < pred_length; i++)
		c->weights[index++] += correction * state[i];
#ifdef QUADRATIC
	// update quadratic coefficients
	for(int i = 0; i < pred_length; i++) {
		for(int j = i; j < pred_length; j++) {
			c->weights[index++] += correction * state[i] * state[j];
		}
	}
#endif
}

double pred_compute(CL *c, double *state)
{
	// first coefficient is offset
	double pre = XCSF_X0 * c->weights[0];
	int index = 1;
	// multiply linear coefficients with the prediction input
	for(int i = 0; i < pred_length; i++)
		pre += c->weights[index++] * state[i];
#ifdef QUADRATIC
	// multiply quadratic coefficients with prediction input
	for(int i = 0; i < pred_length; i++) {
		for(int j = i; j < pred_length; j++) {
			pre += c->weights[index++] * state[i] * state[j];
		}
	}
#endif
	c->pre = pre;
	return pre;
} 

void pred_print(CL *c)
{
	printf("nlms prediction weights: ");
	for(int i = 0; i < c->weights_length; i++)
		printf("%f, ", c->weights[i]);
	printf("\n");
}
#endif
