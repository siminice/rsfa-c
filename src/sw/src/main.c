#include <stdio.h>
#include "parser.h"
//#include "env.h"

int main(int argc, char *argv[])
{
  yyparse();

//  env_free();

  return 0;
}
