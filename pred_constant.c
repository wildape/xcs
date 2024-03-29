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
 * The original XCS constant prediction module.
 */

#ifdef CONSTANT_PREDICTION

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
	pred->pre = INIT_PREDICTION;
	pred->exp = 0;
}
    
void pred_update(PRED *pred, double p, double *state)
{
	pred->exp++;
	if(pred->exp < 1.0/BETA) 
		pred->pre = (pred->pre * (pred->exp-1.0) + p) / (double)pred->exp;
	else
		pred->pre += BETA * (p - pred->pre);

	(void)state; // remove unused parameter warnings
}

double pred_compute(PRED *pred, double *state)
{
	(void)state; // remove unused parameter warnings
	return pred->pre;
}

void pred_print(PRED *pred)
{
	printf("prediction: %f\n", pred->pre);
}

void pred_free(PRED *pred)
{
	// remove unused parameter warnings
	(void)pred;
}

void pred_copy(PRED *to, PRED *from)
{
	to->pre = from->pre;
}

#endif
