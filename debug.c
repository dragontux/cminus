#include "debug.h"

char *debug_strings[] = {
	// eof "type"
	"null",

	// variable types recognized by lexer
	"int",
	"double",
	"string",
	"char",
	"name",

	// control types, also recognized by lexer
	"if",
	"else",
	"while",
	"for",
	"switch",
	"return",

	// punctuation
	"open_paren",
	"open_brack",
	"open_curl",
	"close_paren",
	"close_brack",
	"close_curl",
	"less_than",
	"greater_than",

	"period",
	"comma",
	"semicol",
	"colon",

	"amp",
	"pipe",
	"exclaim",
	"plus",
	"minus",
	"slash",
	"star",

	"equals",

	// parsing types, used for parsing trees
	"program",

	"decl_list",
	"decl",
	"var_decl",
	"fun_decl",
	"var",

	"param_decl_list",
	"param_decl",

	"block",
	"type",
	"statement_list",
	"statement",

	"expr_statement",
	"comp_statement",
	"selecstatement",
	"iter_statement",
	"return_statement",

	"expr",
	"expr_list",
	"expr_list_tail",
	"simple_expr",
	"add_expr",

	"term",
	"factor",
	"call",

	"unary_op",
	"bin_op",

	"add_op",
	"mul_op",
	"rel_op",

	"args_list",

	"endtype",
};

void dump_tree( int level, parse_node_t *token ){
	parse_node_t *move = token;
	int i;

	while( move ){
		printf( "\n" );
		for ( i = 0; i < level; i++ )
			printf( "    " );

		printf( "(%s", debug_strings[ move->type ] );
		if ( move->type == T_NAME )
			printf( " (%s)", (char *)move->data );

		//printf( "\t0x%lx ", (unsigned long)token );

		if ( move->down ){
			dump_tree( level + 1, move->down );
		}

		printf( ")" );

		move = move->next;
	}
}
