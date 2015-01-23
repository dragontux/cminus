#ifndef _parse_h
#define _parse_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cminus/error.h>

typedef enum token_type {
	// EOF type
	T_NULL,

	// Variable types recognized by lexer
	T_INT,
	T_DOUBLE,
	T_STRING,
	T_CHAR,
	T_NAME,

	// Control types, also recognized by lexer
	T_IF,
	T_ELSE,
	T_WHILE,
	T_FOR,
	T_SWITCH,
	T_RETURN,

	// Punctuation
	T_OPEN_PAREN,
	T_OPEN_BRACK,
	T_OPEN_CURL,
	T_CLOSE_PAREN,
	T_CLOSE_BRACK,
	T_CLOSE_CURL,
	T_LESS_THAN,
	T_GREATER_THAN,

	T_PERIOD,
	T_COMMA,
	T_SEMICOL,
	T_COLON,

	T_AMP,
	T_PIPE,
	T_EXCLAIM,
	T_PLUS,
	T_MINUS,
	T_SLASH,
	T_STAR,

	T_EQUALS,

	// Parsing types, used for parsing trees
	T_PROGRAM,

	T_DECL_LIST,
	T_DECL,
	T_VAR_DECL,
	T_FUN_DECL,
	T_VAR,

	T_PARAM_DECL_LIST,
	T_PARAM_DECL,

	T_BLOCK,
	T_TYPE,
	T_STATEMNT_LIST,
	T_STATEMNT,

	T_EXPR_STATEMNT,
	T_COMP_STATEMNT,
	T_SELECT_STATEMNT,
	T_ITER_STATEMNT,
	T_RETURN_STATEMNT,

	T_EXPR,
	T_EXPR_LIST,
	T_EXPR_LIST_TAIL,
	T_SIMPLE_EXPR,
	T_ADD_EXPR,

	T_TERM,
	T_FACTOR,
	T_CALL,

	T_UNARY_OP,
	T_BIN_OP,

	T_ADD_OP,
	T_MUL_OP,
	T_REL_OP,

	T_ARGS_LIST,

	T_ENDTYPE,

} token_type_t;

typedef struct rule {
	token_type_t	type;
	token_type_t	ret;

	struct rule	*next;
	struct rule	*down;
} rule_t;

typedef struct parse_node {
	token_type_t		type;
	unsigned int		status;
	unsigned int		size;
	void 			*data;
	
	struct parse_node 	*next;
	struct parse_node 	*prev;
	struct parse_node 	*up;
	struct parse_node 	*down;

} parse_node_t;

#include "debug.h"

parse_node_t *parse_tokens( parse_node_t * );

/*
parse_node_t *baseline( parse_node_t * );
parse_node_t *maptoken( parse_node_t *, token_type_t );
parse_node_t *reduce( parse_node_t * );
parse_node_t *reduceto( parse_node_t *, token_type_t );
*/
parse_node_t *reduce( parse_node_t *, token_type_t );
token_type_t predict( parse_node_t * );
rule_t *gen_cminus_rules( );

#endif
