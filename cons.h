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
void constants_init();

// values are read from cons.txt
int POP_SIZE;
int NUM_EXPERIMENTS;
int MAX_TRIALS;
double P_CROSSOVER;
double P_MUTATION;
double THETA_SUB;
double EPS_0;
double DELTA;
double THETA_DEL;
double THETA_GA;
double BETA;
double ALPHA; 
double NU;
double GAMMA;
double P_DONTCARE;
char DONT_CARE;
double INIT_PREDICTION;
double INIT_FITNESS;
double INIT_ERROR;
double ERR_REDUC;
double FIT_REDUC;
int TELETRANSPORTATION;
_Bool GA_SUBSUMPTION;
_Bool ACTION_SUBSUMPTION;
int PERF_AVG_TRIALS;
// xcsf computed pred
double XCSF_X0;
double XCSF_ETA;
int XCSF_EXPONENT; // 1 = linear prediction, 2 = quadratic, etc.
// self-adaptive mutation
double muEPS_0;
int NUM_MU;
// set by environment
int pred_length;
int num_actions;
int state_length;
_Bool multi_step;
double max_payoff;
