/* Recursive ascent parser 
 *
 * Implements a LALR parser with mutually recursive 
 * functions instead of a state table.
 *
 * How it works:
 * 	Firstly, the baseline function is called. This is the first stage, aka state,
 * 	and begins the branches of other functions. It pulls the first token
 * 	off the token list, checks it's type, performs any needed actions,
 * 	and returns the appropriate token. If the token it pulls isn't terminal, 
 * 	it calls the first stage of the function branch, which returns one token,
 * 	assuming the token sequence is valid. This returned token is then returned
 * 	from the baseline.
 *
 * 	Each stage of the called branch checks the next token's type, and if the
 * 	next token is valid, calls the next stage in the branch. If the token of 
 * 	the current branch is a valid completion of the branch, any needed actions are
 * 	performed, then the token is returned. This returns it to the first stage of the branch, 
 * 	where a new single token is created with the returning type, and the first token
 *	given to the branch is set to the "down" pointer. This new token's "next"
 *	pointer is set to the last token's "next" pointer, and the last token's
 *	"next" is set to NULL.
 *
 *	After this whole process is done, the start of the token list is set to the return
 *	from baseline. This process loops until either a) There is one token of type "program"
 *	left, or b) there is an error.
 *
 *	The return from "parse_tokens" is a single token tree of type "program".
 *
 */

#include "parse.h"

parse_node_t *parse_tokens( parse_node_t *tokens ){
	parse_node_t *token_list = tokens;

	//while ( token_list && token_list->type != T_PROGRAM )
		token_list = baseline( token_list );

	return token_list;
}

parse_node_t *baseline( parse_node_t *tokens ){
	parse_node_t *ret = NULL;

	if ( !tokens ){
		die( 2, "Have null token, not sure why. Holding note which says \"Look in parser.\"\n" );
		return NULL;
	}

	switch ( tokens->type ){
		case T_PROGRAM:
			ret = tokens;
			break;
		case T_VAR_DECL_LIST:
			break;
		case T_VAR_DECL:
			break;
		case T_FUN_DECL_LIST:
			break;
		case T_FUN_DECL:
			break;
		case T_PARAM_DECL_LIST:
			break;
		case T_PARAM_DECL_LIST_TAIL:
			break;
		case T_PARAM_DECL:
			break;
		case T_BLOCK:
			break;
		case T_TYPE:
			break;
		case T_STATEMNT_LIST:
			break;
		case T_STATEMNT:
			break;
		case T_EXPR:
			break;
		case T_PRIMARY:
			break;
		case T_EXPR_LIST:
			break;
		case T_EXPR_LIST_TAIL:
			break;
		case T_UNARY_OP:
			break;
		case T_BIN_OP:
			break;

		case T_NAME:
			ret = id_stage1( tokens );
			break;

		case T_INT:
		case T_DOUBLE:
		case T_STRING:
		case T_CHAR:
			break;

		default:
			break;
	}
	
	return ret;
}

// Parse names
parse_node_t *id_stage1( parse_node_t *tokens ){
	parse_node_t 	*ret = NULL, 
			*move = NULL;

	if ( !tokens || !tokens->next )
		return tokens; 
	
	switch ( tokens->next->type ){
		case T_NAME:
			move = id_stage2( tokens->next );
			break;

		case T_EQUALS:
			move = id_stage3( tokens->next );
			break;

		case T_OPEN_BRACK:
			move = id_stage4( tokens->next );
			break;

		case T_OPEN_PAREN:
			move = id_stage5( tokens->next );
			break;

		default:
			die( 3, "Expected statement or expression\n" );
			break;
			
	}

	ret = calloc( 1, sizeof( parse_node_t ));

	if ( move ){
		ret = move;
		ret->down = tokens;
	} else {
		ret->down = tokens;
		ret->next = tokens->next;
		ret->type = T_PRIMARY;
		tokens->next = NULL;
	}

	return ret;
}

// At this stage, could be: T_VAR_DECL, T_FUN_DECL, T_PARAM_DECL
parse_node_t *id_stage2( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	switch ( tokens->next->type ){
		case T_SEMICOL:
			move = id_stage2_1( tokens->next );
	}

	return ret;
}

// Returns T_VAR_DECL node
parse_node_t *id_stage2_1( parse_node_t *tokens ){
	parse_node_t *ret = NULL;

	ret = calloc( 1, sizeof( parse_node_t ));

	ret->next = tokens->next;
	ret->type = T_VAR_DECL;
	tokens->next = NULL;

	return ret;
}

parse_node_t *id_stage3( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	return ret;
}

parse_node_t *id_stage4( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	return ret;
}

parse_node_t *id_stage5( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	return ret;
}

// Binary op identity function
parse_node_t *binop( parse_node_t *tokens ){
	parse_node_t *ret = calloc( 1, sizeof( parse_node_t ));

	ret->down = tokens;
	ret->type = T_BIN_OP;
	ret->next = tokens->next;
	tokens->next = NULL;

	return ret;
}










