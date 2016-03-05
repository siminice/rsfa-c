#include <stdlib.h>
#include "normalize.h"
#include "ast.h"

void normalize(struct ast *t)
{
  int tmp;
  struct ast *tmp_ast;

  switch (t->node_type) {
  case AST_ADD:
    normalize(t->node->l);
    normalize(t->node->r);
    switch ((t->node->l->node_type == AST_CONST) << 1 |
            (t->node->r->node_type == AST_CONST)) {
    case 1 << 1 | 1:
      /* c + c -> c */
      tmp = t->node->l->constant
          + t->node->r->constant;
      free(t->node->l);
      free(t->node->r);
      free(t->node);
      t->node_type = AST_CONST;
      t->constant = tmp;
      break;
    case 1 << 1 | 0:
      /* c + f -> f + c */
      tmp_ast = t->node->l;
      t->node->l = t->node->r;
      t->node->r = tmp_ast;
      t->node_type = AST_ADDC;
      break;
    case 0 << 1 | 1:
      /* f + c */
      t->node_type = AST_ADDC;
      break;
    case 0 << 1 | 0:
      /* f + g */
      break;
    };
    break;
  case AST_SUB:
    normalize(t->node->l);
    normalize(t->node->r);
    switch ((t->node->l->node_type == AST_CONST) << 1 |
            (t->node->r->node_type == AST_CONST)) {
    case 1 << 1 | 1:
      /* c - c -> c */
      tmp = t->node->l->constant
          - t->node->r->constant;
      free(t->node->l);
      free(t->node->r);
      free(t->node);
      t->node_type = AST_CONST;
      t->constant = tmp;
      break;
    case 1 << 1 | 0:
      /* c - f -> (-f) + c */
      tmp_ast = malloc(sizeof(struct ast));
      tmp_ast->node_type = AST_MULC;
      tmp_ast->node = malloc(sizeof(struct ast_node));
      tmp_ast->node->r = malloc(sizeof(struct ast));
      tmp_ast->node->r->node_type = AST_CONST;
      tmp_ast->node->r->constant = -1;
      tmp_ast->node->l = t->node->r;
      t->node->r = t->node->l;
      t->node->l = tmp_ast;
      t->node_type = AST_ADDC;
      break;
    case 0 << 1 | 1:
      /* f -c -> f + (-c) */
      t->node->r->constant = -t->node->r->constant;
      t->node_type = AST_ADDC;
      break;
    case 0 << 1 | 0:
      /* f - g -> f + (-g) */
      tmp_ast = malloc(sizeof(struct ast));
      tmp_ast->node_type = AST_MULC;
      tmp_ast->node = malloc(sizeof(struct ast_node));
      tmp_ast->node->r = malloc(sizeof(struct ast));
      tmp_ast->node->r->node_type = AST_CONST;
      tmp_ast->node->r->constant = -1;
      tmp_ast->node->l = t->node->r;
      t->node->r = tmp_ast;
      t->node_type = AST_ADD;
      break;
    };
    break;
  case AST_MUL:
    normalize(t->node->l);
    normalize(t->node->r);
    switch ((t->node->l->node_type == AST_CONST) << 1 |
            (t->node->r->node_type == AST_CONST)) {
    case 1 << 1 | 1:
      /* c * c -> c */
      tmp = t->node->l->constant
          * t->node->r->constant;
      free(t->node->l);
      free(t->node->r);
      free(t->node);
      t->node_type = AST_CONST;
      t->constant = tmp;
      break;
    case 1 << 1 | 0:
      /* c * f -> f * c */
      tmp_ast = t->node->l;
      t->node->l = t->node->r;
      t->node->r = tmp_ast;
      t->node_type = AST_MULC;
      break;
    case 0 << 1 | 1:
      /* f * c */
      t->node_type = AST_MULC;
      break;
    case 0 << 1 | 0:
      /* f * g */
      break;
    };
    break;
  case AST_LT:
    normalize(t->node->l);
    normalize(t->node->r);
    switch ((t->node->l->node_type == AST_CONST) << 1 |
            (t->node->r->node_type == AST_CONST)) {
    case 1 << 1 | 1:
      /* c < c -> c */
      tmp = t->node->l->constant
          < t->node->r->constant;
      free(t->node->l);
      free(t->node->r);
      free(t->node);
      t->node_type = AST_CONST;
      t->constant = tmp;
      break;
    case 1 << 1 | 0:
      /* c < f -> -f < -c */
      tmp_ast = malloc(sizeof(struct ast));
      tmp_ast->node_type = AST_MULC;
      tmp_ast->node = malloc(sizeof(struct ast_node));
      tmp_ast->node->r = malloc(sizeof(struct ast));
      tmp_ast->node->r->node_type = AST_CONST;
      tmp_ast->node->r->constant = -1;
      tmp_ast->node->l = t->node->r;
      t->node->r = t->node->l;
      t->node->r->constant = -t->node->r->constant;
      t->node->l = tmp_ast;
      break;
    case 0 << 1 | 1:
      /* f < c */
      break;
    case 0 << 1 | 0:
      /* f < g -> f-g < 0 */
      tmp_ast = malloc(sizeof(struct ast));
      tmp_ast->node_type = AST_MULC;
      tmp_ast->node = malloc(sizeof(struct ast_node));
      tmp_ast->node->r = malloc(sizeof(struct ast));
      tmp_ast->node->r->node_type = AST_CONST;
      tmp_ast->node->r->constant = -1;
      tmp_ast->node->l = t->node->r;
      t->node->r = tmp_ast;  /* r = -g */
      tmp_ast = malloc(sizeof(struct ast));
      tmp_ast->node_type = AST_ADD;
      tmp_ast->node = malloc(sizeof(struct ast_node));
      tmp_ast->node->l = t->node->l;
      tmp_ast->node->r = t->node->r;
      t->node->l = tmp_ast;  /* l = f + (-g) */
      t->node->r = malloc(sizeof(struct ast));
      t->node->r->node_type = AST_CONST;
      t->node->r->constant = 0;  /* r = 0 */
      break;
    };
    break;
  case AST_VAR:
  case AST_CONST:
  case AST_ADDC:
  case AST_MULC:
    break;
  };
}
