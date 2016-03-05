#ifndef ENV_H
#define ENV_H

/* put variable name in env and return a unique positive id for it
   variable is in interval [lb, gb]
   please note that now env is the "owner" of the string name
   (which will be deleted with env_free) */
int env_put(char *name, int lb, int gb);

/* return the unique id of variable name if it's in env
   and 0 elsewhere
   (lb abd ub allow to retrieve the interval for variable if found) */
int env_is_in(char *name, int *lb, int *ub);

/* return name of variable id
   or NULL if invalid id */
char *env_name(int id, int *lb, int *ub);

/* delete all variables in env */
void env_free();

#endif /* ENV_H */
