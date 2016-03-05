#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
//#include "env.h"

static void rec_print(int offset, struct ast *t)
{
  int i, lb, ub;
  char *name;
  printf("= ");
  for (i = 0; i < offset; ++i)
    printf("  ");
  switch (t->node_type) {
  case AST_VAR:
//    name = env_name(t->var_id, &lb, &ub);
//    printf("%d: %s \tin [%4d, %4d]\n", t->var_id, name, lb, ub);
    break;
  case AST_CONST:
    printf("%d\n", t->constant);
    break;
  case AST_ADD:
    printf("+\n");
    rec_print(offset+1, t->node->l);
    rec_print(offset+1, t->node->r);
    break;
  case AST_ADDC:
    printf("+c\n");
    rec_print(offset+1, t->node->l);
    rec_print(offset+1, t->node->r);
    break;
  case AST_SUB:
    printf("-\n");
    rec_print(offset+1, t->node->l);
    rec_print(offset+1, t->node->r);
    break;
  case AST_MUL:
    printf("*\n");
    rec_print(offset+1, t->node->l);
    rec_print(offset+1, t->node->r);
    break;
  case AST_MULC:
    printf("*c\n");
    rec_print(offset+1, t->node->l);
    rec_print(offset+1, t->node->r);
    break;
  case AST_LT:
    printf("<\n");
    rec_print(offset+1, t->node->l);
    rec_print(offset+1, t->node->r);
    break;
  };
}

void ast_print(struct ast *t)
{
  rec_print(0, t);
}

void ast_free(struct ast* t)
{
  if (t == NULL) return;

  switch (t->node_type) {
  case AST_VAR:
  case AST_CONST:
    free(t);
    break;
  case AST_ADD:
  case AST_ADDC:
  case AST_SUB:
  case AST_MUL:
  case AST_MULC:
  case AST_LT:
    ast_free(t->node->l);
    ast_free(t->node->r);
    free(t->node);
    free(t);
    break;
  };
}
