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

void act_init(ACT *act)
{
	// remove unused parameter warnings
	(void)act;
}

void act_free(ACT *act)
{
	// remove unused parameter warnings
	(void)act;
}

void act_copy(ACT *to, ACT *from)
{
	to->a = from->a;
}    

void act_rand(ACT *act)
{
	act->a = irand(0, num_actions);
}

void act_cover(ACT *act, char *state, int i)
{
	act->a = i;
	// remove unused parameter warnings
	(void)state;
}

_Bool act_duplicate(ACT *act1, ACT *act2)
{
	if(act1->a == act2->a)
		return true;
	else
		return false;
}
             
_Bool act_mutate(ACT *act)
{
	_Bool mod = false;
	if(drand() < P_MUTATION) {
		int a = 0;
		do {
			a = irand(0, num_actions);
		} while(a == act->a);
		act->a = a;
		mod = true;
	}
	return mod;
}
 
void act_print(ACT *act)
{
	printf("action = %d\n", act->a);
}
