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
 * Classifier integer actions module.
 *
 * Represents a classifier's executable action as an integer. Provides
 * functions to generate random actions and to mutate an action, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cons.h"
#include "random.h"
#include "cl.h"

void act_init(CL *c)
{
	// remove unused parameter warnings
	(void)c;
}

void act_free(CL *c)
{
	// remove unused parameter warnings
	(void)c;
}

void act_copy(CL *to, CL *from)
{
	to->act = from->act;
}    

void act_rand(CL *c)
{
	c->act = irand(0, num_actions);
}
             
_Bool act_mutate(CL *c)
{
	_Bool mod = false;
	if(drand() < P_MUTATION) {
		int act = 0;
		do {
			act = irand(0, num_actions);
		} while(act == c->act);
		c->act = act;
		mod = true;
	}
	return mod;
}
 
void act_print(CL *c)
{
	printf("action = %d\n", c->act);
}
