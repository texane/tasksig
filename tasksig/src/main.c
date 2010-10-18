#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "sparse/lib.h"
#include "sparse/allocate.h"
#include "sparse/token.h"
#include "sparse/parse.h"
#include "sparse/symbol.h"
#include "sparse/expression.h"
#include "sparse/linearize.h"


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

#if 0

static void examine_argument(struct symbol* sym)
{
  char name_buffer[256];
  char type_buffer[256];

  strcpy(name_buffer, show_ident(sym->ident));
  strcpy(type_buffer, get_root_type(sym));

  printf(" %s %s\n", type_buffer, name_buffer);
}

static void examine_function(struct symbol* sym)
{
  struct symbol* arg;
  FOR_EACH_PTR(sym->arguments, arg) {
    examine_argument(arg);
  } END_FOR_EACH_PTR(arg);
}

#endif


static void examine_expression(struct expression*);

static void examine_assignment(struct expression* expr)
{
  if (!expr || !expr->ctype) return ;

  printf("assignment left = right\n");

  examine_expression(expr->left);
  examine_expression(expr->right);
}

static void examine_preop(struct expression* expr)
{
  printf("preop_operation: %x\n", expr->op);

  /* examine address generation */
  if (expr->op == '*')
    examine_expression(expr->unop);
}

static void examine_binop(struct expression* expr)
{
  printf("binop(%c)\n", expr->op);
  examine_expression(expr->left);
  examine_expression(expr->right);
}

static void examine_symbol_expr(struct symbol* sym)
{
  printf("symbol(%s)\n", show_ident(sym->ident));
}

static void examine_expression(struct expression* expr)
{
  if (!expr) return ;

  switch (expr->type)
  {
  case EXPR_ASSIGNMENT:
    examine_assignment(expr);
    break;

  case EXPR_BINOP:
  case EXPR_LOGICAL:
    examine_binop(expr);
    break;

  case EXPR_SYMBOL:
    examine_symbol_expr(expr->symbol);
    break;

  case EXPR_VALUE:
    printf("value\n");
    break;

  case EXPR_STATEMENT:
    printf("statement\n");
    break;

  case EXPR_PREOP:
    printf("preop\n");
    examine_preop(expr);
    break;

  case EXPR_POSTOP:
    printf("postop\n");
    break;

  case EXPR_CAST:
  case EXPR_FORCE_CAST:
  case EXPR_IMPLIED_CAST:
    printf("cast\n");
    examine_expression(expr->cast_expression);
    break;

  default:
    printf("unknwon_expr(%u)\n", expr->type);
    break;
  }
}

static void examine_statement(struct statement* stmt)
{
  if (!stmt) return ;

  switch (stmt->type)
  {
  case STMT_EXPRESSION:
    printf("expression\n");
    examine_expression(stmt->expression);
    break;

  case STMT_DECLARATION:
    printf("decl\n");
    /* show_symbol_decl(stmt->declaration); */
    break;

  case STMT_RETURN:
    printf("return\n");
    /* todo: could be return ptr where ptr a memory alias */
    /* return show_return_stmt(stmt); */
    break ;

  case STMT_ITERATOR:
    printf("iterator\n");
    /* todo: show-parse.c */
    break;

  case STMT_RANGE:
    printf("range\n");
    /* todo: show-parse.c */
    break;

  case STMT_COMPOUND:
    {
      struct statement* child;
      FOR_EACH_PTR(stmt->stmts, child) {
	examine_statement(child);
      } END_FOR_EACH_PTR(child);
      break;
    }

  default:
    {
      printf("unknown_statement\n");
      break;
    }
  }
}

static void examine_symbol(struct symbol* sym)
{
  switch (sym->type)
  {
  case SYM_NODE:
    if (sym->ctype.base_type->type == SYM_FN)
      examine_symbol(sym->ctype.base_type);
    break;

  case SYM_FN:
    printf("---- function\n");
    examine_statement(sym->stmt);
    /* show_statement(sym->stmt); */
    break;
  }
}

static void examine_insn(struct instruction* insn)
{
  if (insn->target != NULL)
    if (insn->target->ident != NULL)
      printf("target: %u %s\n", insn->symbol->type, show_ident(insn->target->ident));

  switch(insn->opcode) {
  case OP_BR:
#if 0
    printf("br");
    if (insn->cond)
    {
      printf(", t: %u", insn->cond->type);
      printf(", cond %s", show_ident(insn->cond->ident));
    }
    printf("\n");
#endif
    break;

  case OP_STORE:
#if 0
    if (insn->symbol->type == PSEUDO_SYM) {
      printf("store %s\n", show_ident(insn->symbol->sym->ident));
    }
#endif
    break;

  case OP_LOAD:
#if 0
    if (insn->symbol->type == PSEUDO_SYM) {
      printf("load %s\n", show_ident(insn->symbol->sym->ident));
    }
#endif
    break;

  case OP_ADD:
    {
      char buffers[3][64];

      *buffers[0] = 0;
      *buffers[1] = 0;
      *buffers[2] = 0;

      if (insn->target) strcpy(buffers[0], show_ident(insn->target->ident));
      if (insn->src1) strcpy(buffers[1], show_ident(insn->src1->ident));
      if (insn->src2) strcpy(buffers[2], show_ident(insn->src2->ident));

      printf("add: %s = %s + %s\n", buffers[0], buffers[1], buffers[2]);
    }
    break;

  default:
    /* printf("opcode %u\n", insn->opcode); */
    break; 
  }
}

static void examine_ep(struct entrypoint* ep)
{
  /* basic block level analysis */
  struct basic_block *bb;
  FOR_EACH_PTR(ep->bbs, bb) {
    /* List loads and stores */
    struct instruction *insn;
    FOR_EACH_PTR(bb->insns, insn) {
      examine_insn(insn);
    } END_FOR_EACH_PTR(insn);
  } END_FOR_EACH_PTR(bb);
}

#if 0 /* unused */

/* Insert edges for intra- or inter-file calls, depending on the value
 * of internal. Bold edges are used for calls with destinations;
 * dashed for calls to external functions */
static void graph_calls(struct entrypoint *ep, int internal)
{
	struct basic_block *bb;
	struct instruction *insn;

	const char *fname, *sname;

	fname = show_ident(ep->name->ident);
	sname = stream_name(ep->entry->bb->pos.stream);

	FOR_EACH_PTR(ep->bbs, bb) {
		if (!bb)
			continue;
		if (!bb->parents && !bb->children && !bb->insns && verbose < 2)
			continue;

		FOR_EACH_PTR(bb->insns, insn) {
			if (insn->opcode == OP_CALL &&
			    internal == !(insn->func->sym->ctype.modifiers & MOD_EXTERN)) {

				/* Find the symbol for the callee's definition */
				struct symbol * sym;
				if (insn->func->type == PSEUDO_SYM) {
					for (sym = insn->func->sym->ident->symbols;
					     sym; sym = sym->next_id) {
						if (sym->namespace & NS_SYMBOL && sym->ep)
							break;
					}

					if (sym)
						printf("bb%p -> bb%p"
						       "[label=%d,line=%d,col=%d,op=call,style=bold,weight=30];\n",
						       bb, sym->ep->entry->bb,
						       insn->pos.line, insn->pos.line, insn->pos.pos);
					else
						printf("bb%p -> \"%s\" "
						       "[label=%d,line=%d,col=%d,op=extern,style=dashed];\n",
						       bb, show_pseudo(insn->func),
						       insn->pos.line, insn->pos.line, insn->pos.pos);
				}
			}
		} END_FOR_EACH_PTR(insn);
	} END_FOR_EACH_PTR(bb);
}

#endif

int main(int argc, char **argv)
{
  struct string_list *filelist = NULL;
  char *file;
  struct symbol *sym;

  struct symbol_list *fsyms, *all_syms=NULL;

  fsyms = sparse_initialize(argc, argv, &filelist);
  concat_symbol_list(fsyms, &all_syms);

  /* Linearize all symbols, graph internal basic block
   * structures and intra-file calls */
  FOR_EACH_PTR_NOTAG(filelist, file) {

    fsyms = sparse(file);
    concat_symbol_list(fsyms, &all_syms);

    FOR_EACH_PTR(fsyms, sym) {
      expand_symbol(sym);
      linearize_symbol(sym);
    } END_FOR_EACH_PTR(sym);

    FOR_EACH_PTR(fsyms, sym) {
      if (sym->ep) {
	const char* fname = show_ident(sym->ep->name->ident);
	const char* sname = stream_name(sym->ep->entry->bb->pos.stream);
	printf("%s::%s\n", sname, fname);
	examine_ep(sym->ep);
      }

      examine_symbol(sym);

    } END_FOR_EACH_PTR(sym);

  } END_FOR_EACH_PTR_NOTAG(file);

#if 0 /* unused */
  /* Graph inter-file calls */
  FOR_EACH_PTR(all_syms, sym) {
    if (sym->ep)
      graph_calls(sym->ep, 0);
  } END_FOR_EACH_PTR_NOTAG(sym);
#endif

  return 0;
}
