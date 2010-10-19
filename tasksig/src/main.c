#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sparse/lib.h"
#include "sparse/allocate.h"
#include "sparse/token.h"
#include "sparse/parse.h"
#include "sparse/symbol.h"
#include "sparse/expression.h"
#include "sparse/linearize.h"


/* debugging macros
 */

#define DEBUG_ASSERT(__expr) \
  if (!(__expr)) { printf("!(" #__expr ")\n"); exit(-1); }

#define DEBUG_PRINTF(__s, ...)	\
  printf(__s, ##__VA_ARGS__)


/* symbol references list
 */

struct symrefs
{
  /* top referenced symbol */
  struct symbol* top;
  /* syms referencing top  */
  struct symbol_list* refs;
};

DECLARE_PTR_LIST(symrefs_list, struct symrefs);

static inline void init_symrefs(struct symrefs* symrefs)
{
  symrefs->top = NULL;
  symrefs->refs = NULL;
}

static struct symrefs* new_symrefs(void)
{
  struct symrefs* const symrefs = malloc(sizeof(struct symrefs));
  if (symrefs == NULL) return NULL;
  init_symrefs(symrefs);
  return symrefs;
}


/* ast visitor type
 */

struct visitor
{
  /* maintains the state of the ast visitor */

  /* entry function */
  const char* entry_name;
  struct symbol_list* entry_args;

  /* symbol references */
  struct symrefs_list* symrefs;
};

static void init_visitor(struct visitor* viz, const char* entry_name)
{
  viz->entry_name = entry_name;
  viz->symrefs = NULL;
}

static void fini_visitor(struct visitor* viz)
{
  /* todo: release symrefs list memory */
}


static const char* get_root_type(struct symbol* sym)
{
  if (sym == NULL)
    return "unknown_type";

  if (sym->type == SYM_NODE)
  {
  }
  else if (sym->type == SYM_PTR)
  {
    printf("* ");
  }
  else if (sym->type == SYM_BASETYPE)
  {
    return builtin_typename(sym);
  }
  else if (sym->type == SYM_KEYWORD)
  {
    if (sym->ctype.modifiers & MOD_UNSIGNED)
      printf("unsigned ");
  }
  else if (sym->type == SYM_STRUCT)
  {
    return show_ident(sym->ident);
  }
  else if (sym->type == SYM_STRUCT)
  {
    return show_ident(sym->ident);
  }
  else if (sym->type == SYM_ARRAY)
  {
    return "array ";
  }

  return get_root_type(sym->ctype.base_type);
}

static void visit_argument(struct visitor* viz, struct symbol* sym)
{
  char name_buffer[256];
  char type_buffer[256];

  strcpy(name_buffer, show_ident(sym->ident));
  strcpy(type_buffer, get_root_type(sym));

  printf(" %s %s\n", type_buffer, name_buffer);
}

static void visit_function(struct visitor* viz, struct symbol* sym)
{
  struct symbol* arg;
  FOR_EACH_PTR(sym->arguments, arg) {
    visit_argument(viz, arg);
  } END_FOR_EACH_PTR(arg);
}

static void visit_expression(struct visitor*, struct expression*);

static void visit_assignment(struct visitor* viz, struct expression* expr)
{
  if (!expr || !expr->ctype) return ;

  printf("assignment left = right\n");

  visit_expression(viz, expr->left);
  visit_expression(viz, expr->right);
}

static void visit_preop(struct visitor* viz, struct expression* expr)
{
  printf("preop_operation: %x\n", expr->op);

  /* visit address generation */
  if (expr->op == '*')
    visit_expression(viz, expr->unop);
}

static void visit_binop(struct visitor* viz, struct expression* expr)
{
  printf("binop(%c)\n", expr->op);
  visit_expression(viz, expr->left);
  visit_expression(viz, expr->right);
}

static void visit_symbol_expr(struct visitor* viz, struct symbol* sym)
{
  printf("symbol(%s)\n", show_ident(sym->ident));
}

static void visit_expression(struct visitor* viz, struct expression* expr)
{
  if (!expr) return ;

  switch (expr->type)
  {
  case EXPR_ASSIGNMENT:
    visit_assignment(viz, expr);
    break;

  case EXPR_BINOP:
  case EXPR_LOGICAL:
    visit_binop(viz, expr);
    break;

  case EXPR_SYMBOL:
    visit_symbol_expr(viz, expr->symbol);
    break;

  case EXPR_VALUE:
    printf("value\n");
    break;

  case EXPR_STATEMENT:
    printf("statement\n");
    break;

  case EXPR_PREOP:
    printf("preop\n");
    visit_preop(viz, expr);
    break;

  case EXPR_POSTOP:
    printf("postop\n");
    break;

  case EXPR_CAST:
  case EXPR_FORCE_CAST:
  case EXPR_IMPLIED_CAST:
    printf("cast\n");
    visit_expression(viz, expr->cast_expression);
    break;

  case EXPR_CALL:
    {
      printf("call\n");
      break;
    } 

  default:
    printf("unknwon_expr(%u)\n", expr->type);
    break;
  }
}

static void visit_statement(struct visitor* viz, struct statement* stmt)
{
  if (!stmt) return ;

  switch (stmt->type)
  {
  case STMT_EXPRESSION:
    DEBUG_PRINTF("expression ");
    visit_expression(viz, stmt->expression);
    break;

  case STMT_DECLARATION:
    DEBUG_PRINTF("decl ");
    /* show_symbol_decl(stmt->declaration); */
    break;

  case STMT_RETURN:
    DEBUG_PRINTF("return ");
    /* todo: could be return ptr where ptr a memory alias */
    /* return show_return_stmt(stmt); */
    break ;

  case STMT_ITERATOR:
    printf("iterator ");
    visit_statement(viz, stmt->iterator_statement);
    /* todo: show-parse.c */
    break;

  case STMT_RANGE:
    DEBUG_PRINTF("range ");
    /* todo: show-parse.c */
    break;

  case STMT_COMPOUND:
    {
      struct statement* child;
      FOR_EACH_PTR(stmt->stmts, child) {
	visit_statement(viz, child);
      } END_FOR_EACH_PTR(child);
      break;
    }

  default:
    DEBUG_PRINTF("unknown_statement %u ", stmt->type);
    break;
  }
}

static void visit_symbol(struct visitor* viz, struct symbol* sym)
{
  switch (sym->type)
  {
  case SYM_NODE:
    if (sym->ctype.base_type->type == SYM_FN)
      visit_symbol(viz, sym->ctype.base_type);
    break;

  case SYM_FN:
    visit_function(viz, sym);
    visit_statement(viz, sym->stmt);
    break;
  }
}

static inline struct symbol* next_function_symbol(struct symbol* sym)
{
  if (sym->type != SYM_NODE)
    return NULL;
  if (sym->ctype.base_type->type != SYM_FN)
    return NULL;
  return sym->ctype.base_type;
}

static int visit_root_entrypoint(struct visitor* viz, struct symbol* sym)
{
  if (sym->ep == NULL)
    return -1;

  DEBUG_ASSERT(sym->ep->name && sym->ep->name->ident);
  if (strcmp(viz->entry_name, show_ident(sym->ep->name->ident)))
    return -1;

  if ((sym = next_function_symbol(sym)) != NULL)
  {
    struct symbol* arg;
    FOR_EACH_PTR(sym->arguments, arg) {
      struct symrefs* const symrefs = new_symrefs();
      DEBUG_ASSERT(symrefs);
      symrefs->top = arg;
      DEBUG_PRINTF("tracking(%s)\n", show_ident(arg->ident));
      add_ptr_list(&viz->symrefs, symrefs);
    } END_FOR_EACH_PTR(arg);

    DEBUG_ASSERT(sym->stmt);
    visit_statement(viz, sym->stmt);
  }

  return 0;
}

int main(int argc, char **argv)
{
  struct string_list *filelist = NULL;
  char *file;
  struct symbol *sym;

  struct symbol_list *fsyms, *all_syms=NULL;

  fsyms = sparse_initialize(argc, argv, &filelist);
  concat_symbol_list(fsyms, &all_syms);
  dbg_dead = 1;

  /* initialize the visitor */
  struct visitor viz;
  init_visitor(&viz, "entry");

  FOR_EACH_PTR_NOTAG(filelist, file) {
    fsyms = sparse(file);

    FOR_EACH_PTR(fsyms, sym) {
      expand_symbol(sym);
      linearize_symbol(sym);
    } END_FOR_EACH_PTR(sym);

    FOR_EACH_PTR(fsyms, sym) {
      if (visit_root_entrypoint(&viz, sym) != -1)
	break ;
    } END_FOR_EACH_PTR(sym);

  } END_FOR_EACH_PTR_NOTAG(file);

  fini_visitor(&viz);

  return 0;
}
