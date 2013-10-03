#include "debug.h"

char *debug_strings[] = {
	// EOF type
	"T_NULL",

	// Variable types recognized by lexer
	"T_INT",
	"T_DOUBLE",
	"T_STRING",
	"T_CHAR",
	"T_NAME",

	// Control types, also recognized by lexer
	"T_IF",
	"T_ELSE",
	"T_WHILE",
	"T_FOR",
	"T_SWITCH",
	"T_RETURN",

	// Punctuation
	"T_OPEN_PAREN",
	"T_OPEN_BRACK",
	"T_OPEN_CURL",
	"T_CLOSE_PAREN",
	"T_CLOSE_BRACK",
	"T_CLOSE_CURL",
	"T_LESS_THAN",
	"T_GREATER_THAN",

	"T_PERIOD",
	"T_COMMA",
	"T_SEMICOL",
	"T_COLON",

	"T_AMP",
	"T_PIPE",
	"T_EXCLAIM",
	"T_PLUS",
	"T_MINUS",
	"T_SLASH",
	"T_STAR",

	"T_EQUALS",

	// Parsing types, used for parsing trees
	"T_PROGRAM",

	"T_VAR_DECL_LIST",
	"T_VAR_DECL",

	"T_FUN_DECL_LIST",
	"T_FUN_DECL",

	"T_PARAM_DECL_LIST",
	"T_PARAM_DECL_LIST_TAIL",
	"T_PARAM_DECL",

	"T_BLOCK",
	"T_TYPE",
	"T_STATEMNT_LIST",
	"T_STATEMNT",
	"T_PRIMARY",
	"T_EXPR",
	"T_EXPR_LIST",
	"T_EXPR_LIST_TAIL",

	"T_UNARY_OP",
	"T_BIN_OP",

	"T_ENDTYPE",

};

void dump_tree( int level, parse_node_t *token ){
	parse_node_t *move = token;
	int i;

	while( move ){
		for ( i = 0; i < level; i++ )
			printf( "\t" );

		//printf( "%d\n", move->type );
		printf( "%s\n", debug_strings[ move->type ] );
		if ( move->down )
			dump_tree( level + 1, move->down );

		move = move->next;
	}
}

