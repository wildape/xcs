void env_init(char **argv);
void env_free();
double env_exec_action(int action);
char *env_get_state();
_Bool env_is_reset();
void env_reset();
double *env_get_dstate();
