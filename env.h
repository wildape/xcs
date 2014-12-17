void init_env(char **argv);
double execute_action(int action);
char *get_state();
_Bool is_reset();
void env_reset();
#ifdef XCSF
double *get_dstate();
#endif
