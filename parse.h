#ifndef _parse_h
#define _parse_h
#include <stdio.h>
#include <stdlib.h>

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

	T_VAR_DECL_LIST,
	T_VAR_DECL,

	T_FUN_DECL_LIST,
	T_FUN_DECL,

	T_PARAM_DECL_LIST,
	T_PARAM_DECL_LIST_TAIL,
	T_PARAM_DECL,

	T_BLOCK,
	T_TYPE,
	T_STATEMNT_LIST,
	T_STATEMNT,
	T_PRIMARY,
	T_EXPR,
	T_EXPR_LIST,
	T_EXPR_LIST_TAIL,

	T_UNARY_OP,
	T_BIN_OP,

	T_ENDTYPE,

} token_type_t;

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
parse_node_t *baseline( parse_node_t * );
parse_node_t *reduce( parse_node_t * );

// Grammar functions 
parse_node_t *block_stage1( parse_node_t * );

parse_node_t *funcdecl_stage1( parse_node_t * );

parse_node_t *id_stage1( parse_node_t * );

parse_node_t *id_stage2( parse_node_t * );
parse_node_t *id_stage2_1( parse_node_t * );
parse_node_t *id_stage2_2( parse_node_t * );
parse_node_t *id_stage2_2_1( parse_node_t * );
parse_node_t *id_stage2_3( parse_node_t * );

parse_node_t *id_stage3( parse_node_t * );
parse_node_t *id_stage4( parse_node_t * );
parse_node_t *id_stage5( parse_node_t * );

parse_node_t *vardecl_stage1( parse_node_t * );

#endif
