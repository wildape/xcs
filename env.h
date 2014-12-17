void env_init(char **argv);
double env_exec_action(int action);
char *env_get_state();
_Bool env_is_reset();
void env_reset();
#ifdef XCSF
double *env_get_dstate();
#endif
