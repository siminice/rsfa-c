#ifndef NORMALIZE_H
#define NORMALIZE_H

#include "ast.h"

/* normalize the AST t,
   after this t contains only: 
   - AST_VAR
   - AST_CONST 
   - AST_ADD, two children are not AST_CONST
   - AST_ADDC, right children is AST_CONST (and not left)
   - AST_MUL, two children are not AST_CONST
   - AST_MULC, right children is AST_CONST (and not left)
   - AST_LT, right children is AST_CONST (and not left) */
void normalize(struct ast *t);

#endif /* NORMALIZE_H */
