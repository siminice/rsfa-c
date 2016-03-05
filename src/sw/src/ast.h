#ifndef AST_H
#define AST_H

typedef enum {AST_VAR, AST_CONST,
              AST_ADD, AST_SUB, AST_MUL, AST_LT,
              /* operations with a constant,
                 constant is right child
                 after applying normalization (see normalize.h) */
              AST_ADDC, AST_MULC} t_ast_ops;

struct ast;

struct ast_node {
  struct ast *l;
  struct ast *r;
};

struct ast {
  t_ast_ops node_type;
  union {
    struct ast_node *node;
    int var_id;
    int constant;
  };
};

/* print an AST */
void ast_print(struct ast*);

/* free the AST t */
void ast_free(struct ast* t);

#endif  /* AST_H */
