#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "env.h"

#define ENV_SIZE 1024

/* var name is in the interval [lb, ub] */
struct var {
  char *name;
  int lb;
  int ub;
};

static struct var env[ENV_SIZE];
static int nb_var=0;

int env_put(char *name, int lb, int ub)
{
  ++nb_var;

  if (nb_var >= ENV_SIZE) {
    fprintf(stderr, "Error: environnement plein (ENV_SIZE = %d).\n", ENV_SIZE);
    exit(1);
  }

  env[nb_var].name = name;
  env[nb_var].lb = lb;
  env[nb_var].ub = ub;
    
  return nb_var;
}

int  env_is_in(char *name, int *lb, int *ub)
{
  int i;

  /* probably not the better way to do it
     (should use a hash table instead) but still works */
  for (i = 1; i <= nb_var; ++i)
    if (strcmp(name, env[i].name) == 0) {
      if (lb != NULL) *lb = env[i].lb;
      if (ub != NULL) *ub = env[i].ub;
      return i;
    }

  return 0;  /* not found */
}

char *env_name(int id, int *lb, int *ub)
{
  if (id < 1 || id > nb_var)
    return NULL;

  if (lb != NULL) *lb = env[id].lb;
  if (ub != NULL) *ub = env[id].ub;
  return env[id].name;
}

void env_free()
{
  while (nb_var) {
    free(env[nb_var].name);
    --nb_var;
  }
}
