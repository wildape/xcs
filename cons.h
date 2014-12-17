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
 * ************
 * Description:
 * ************
 *
 * XCS global constants; read from cons.txt
 */

void constants_init(int argc, char **argv);

// experiment parameters
_Bool POP_INIT; // population initially empty or filled with random conditions
int MAX_TRIALS; // number of problem instances to run in one experiment
int NUM_EXPERIMENTS; // number of experiments to run
int PERF_AVG_TRIALS; // number of problem instances to average performance output
int POP_SIZE; // maximum number of macro-classifiers in the population
// classifier parameters
double ALPHA; // linear coefficient used in calculating classifier accuracy
double BETA; // learning rate for updating error, fitness, and set size
double DELTA; // fit used in prob of deletion if fit less than this frac of avg pop fit 
double EPS_0; // classifier target error, under which the fitness is set to 1
double ERR_REDUC; // amount to reduce an offspring's error
double FIT_REDUC; // amount to reduce an offspring's fitness
double GAMMA; // discount factor in calculating the reward for multi-step problems
double INIT_ERROR; // initial classifier error value
double INIT_FITNESS; // initial classifier fitness value
double NU; // exponent used in calculating classifier accuracy
double THETA_DEL; // min experience before fitness used in probability of deletion
int TELETRANSPORTATION; // num steps to reset a multi-step problem if goal not found
// genetic algorithm parameters
double P_CROSSOVER; // probability of applying crossover (for hyperrectangles)
double P_MUTATION; // probability of mutation occuring per allele
double THETA_GA; // average match set time between GA invocations
// self-adaptive mutation parameters
double muEPS_0; // minimum value of a self-adaptive mutation rate
int NUM_MU; // number of self-adaptive mutation rates
// classifier condition parameters
char DONT_CARE; // symbol used for ternary condition
double P_DONTCARE; // per allele probability of don't care in covering or random init
// prediction parameters
double INIT_PREDICTION; // initial prediction value for XCS constant prediction
double XCSF_ETA; // learning rate for updating the computed prediction
double XCSF_X0; // prediction weight vector offset value
// subsumption parameters
_Bool ACTION_SUBSUMPTION; // whether to subsume more specific rules in action set
_Bool GA_SUBSUMPTION; // whether to try and subsume offspring classifiers
_Bool SET_SUBSUMPTION; // whether to perform match set subsumption
double THETA_SUB; // minimum experience of a classifier to become a subsumer
// set by environment
_Bool multi_step; // whether the problem is single or multi-step
double max_payoff; // maximum environment payoff for executing an action
int num_actions; // number of executable actions a classifier can make
int pred_length; // number of real-value input variables to compute prediction
int state_length; // number of binary input variables
