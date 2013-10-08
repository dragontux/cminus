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
extern char *debug_strings[];

parse_node_t *parse_tokens( parse_node_t *tokens ){
	parse_node_t *token_list = tokens;

	token_list = reduce( token_list );

	return token_list;
}

parse_node_t *baseline( parse_node_t *tokens ){
	parse_node_t *ret = tokens;

	if ( !tokens ){
		printf( "[parser] Have null tokens list\n" );
		return NULL;
	}

	switch ( tokens->type ){
		case T_PROGRAM:
			ret = tokens;
			break;
		/*
		case T_VAR_DECL_LIST:
			break;
		*/
		case T_VAR_DECL:
			ret = vardecl_stage1( tokens );
			break;
		/*
		case T_FUN_DECL_LIST:
			break;
		*/
		case T_FUN_DECL:
			ret = funcdecl_stage1( tokens );
			break;
		/*
		case T_PARAM_DECL_LIST:
			break;
		case T_PARAM_DECL_LIST_TAIL:
			break;
		*/
		case T_PARAM_DECL:
			ret = paramdecl_stage1( tokens );
			break;

		case T_OPEN_CURL:
			ret = block_stage1( tokens );
			break;

		case T_SEMICOL:
			ret = maptoken( tokens, T_STATEMNT );
			break;

		case T_EXPR:
			ret = expr_stage1( tokens );
			break;

		case T_STATEMNT:
			ret = statelist_stage1( tokens );
			break;
		/*
		case T_TYPE:
			break;
		case T_STATEMNT_LIST:
			break;
		case T_EXPR_LIST:
			break;
		case T_EXPR_LIST_TAIL:
			break;
		case T_UNARY_OP:
			break;
		case T_BIN_OP:
			break;
		*/

		case T_BLOCK:
			ret = maptoken( tokens, T_STATEMNT );
			break;

		case T_NAME:
			ret = id_stage1( tokens );
			break;

		case T_RETURN:
			ret = return_stage1( tokens );
			break;

		case T_WHILE:
			ret = while_stage1( tokens );
			break;

		case T_PRIMARY:
			ret = maptoken( tokens, T_EXPR );
			break;

		case T_AMP:
		case T_PIPE:
		case T_PLUS:
		case T_MINUS:
		case T_SLASH:
		case T_STAR:
			ret = maptoken( tokens, T_BIN_OP );
			break;

		case T_INT:
		case T_DOUBLE:
		case T_STRING:
		case T_CHAR:
			ret = maptoken( tokens, T_EXPR );
			break;

		default:
			printf( "[baseline] returned self\n" );
			ret = tokens;
			break;
	}
	
	return ret;
}

// Run series of tokens through baseline repeatedly until a terminal type is reached
parse_node_t *reduce( parse_node_t *tokens ){
	parse_node_t *temp, *ret;

	if ( !tokens )
		return tokens;

	printf( "[reduce] reducing %s... ", debug_strings[tokens->type] );
	ret = temp = tokens;
	while (( ret = baseline( temp )) && ret != temp )
		temp = ret;

	ret = ret? ret : temp;
	printf( "reduced to %s ", debug_strings[ret->type] );

	printf( "\n" );
	return ret;
}

// same as reduce, except that the loop is halted if a specific type is reached first.
parse_node_t *reduceto( parse_node_t *tokens, token_type_t type ){
	parse_node_t *temp, *ret;

	if ( !tokens )
		return tokens;

	printf( "[reduce] reducing %s... ", debug_strings[tokens->type] );
	ret = temp = tokens;
	while (( ret = baseline( temp )) && ret != temp && ret->type != type )
		temp = ret;

	ret = ret? ret : temp;

	/*
	while ( ret->type == type && temp != ret )
		temp = ret = baseline( ret );
	*/

	printf( "reduced to %s ", debug_strings[ret->type] );

	printf( "\n" );
	return ret;
}

// Parse blocks
parse_node_t *block_stage1( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	printf( "[block_stage1]\n" );
	ret = calloc( 1, sizeof( parse_node_t ));
	move = reduce( tokens->next );

	ret->type = T_BLOCK;
	ret->down = move;

	move = block_stage2( move );

	if ( !move->next || move->next->type != T_CLOSE_CURL ){
		dump_tree( 0, move->next );
		die( 2, "Expected closing brace\n" );
	} else {
		move = move->next;
	}

	ret->next = move->next;
	move->next = NULL;

	return ret;
}

parse_node_t *block_stage2( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;
	printf( "[block_stage2] %s\n", debug_strings[ tokens->next->type ] );

	//tokens = tokens->next;

	switch( tokens->type ){
		case T_VAR_DECL_LIST:
			ret = block_stage3( tokens );
			break;
		case T_STATEMNT_LIST:
			ret = block_stage4( tokens );
			break;
		default:
			dump_tree( 0, tokens );
			die( 2, "Expected variable declaration or statement\n" );
	}

	return ret;
}

parse_node_t *block_stage3( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;
	printf( "[block_stage3] %s\n", debug_strings[ tokens->next->type ] );
	
	if ( !tokens->next || tokens->next->type != T_STATEMNT_LIST ){
		dump_tree( 0, tokens );
		die( 2, "Expected statement after variable declaration\n" );
	}

	ret = block_stage4( tokens->next );

	return ret;
}

parse_node_t *block_stage4( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;
	printf( "[block_stage3]\n" );

	ret = tokens;

	return ret;
}

parse_node_t *expr_stage1( parse_node_t *tokens ){
	printf( "[expr_stage1] %s\n", debug_strings[tokens->next->type] );
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	if ( tokens->next->type != T_SEMICOL )
		tokens->next = baseline( tokens->next );

	switch( tokens->next->type ){
		case T_SEMICOL:
			move = expr_stage2( tokens );
			break;

		case T_BIN_OP:
			move = expr_stage3( tokens );
			break;

		default:
			ret = tokens;
			break;
	}

	if ( !ret ){
		ret = calloc( 1, sizeof( parse_node_t ));
		ret->down = tokens;
		ret->type = move->status;
		ret->next = move->next;
		move->next = NULL;
	}

	return ret;
}

parse_node_t *expr_stage2( parse_node_t *tokens ){
	printf( "[expr_stage2]\n" );
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	tokens = tokens->next;
	printf( "\t%s\n", debug_strings[tokens->type] );

	//tokens->next->status = T_STATEMNT;
	ret = tokens;
	ret->status = T_STATEMNT;

	return ret;
}

parse_node_t *expr_stage3( parse_node_t *tokens ){
	printf( "[expr_stage3] " );
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	tokens = tokens->next;

	printf( "%s\n", debug_strings[ tokens->next->type ]);

	tokens->next = reduceto( tokens->next, T_EXPR );

	if ( tokens->next->type != T_EXPR ){
		dump_tree( 0, tokens );
		die( 2, "Expected expression after binary operator\n" );
	}
	

	ret = tokens->next;
	ret->status = T_EXPR;

	return ret;
}

// Maps a single token to another token
parse_node_t *maptoken( parse_node_t *tokens, token_type_t type ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	ret = calloc( 1, sizeof( parse_node_t ));

	ret->type = type;
	ret->down = tokens;
	ret->next = tokens->next;
	tokens->next = NULL;

	return ret;
}

// Parse function declaration lists
parse_node_t *funcdecl_stage1( parse_node_t *tokens ){
	parse_node_t 	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	if ( tokens->next->type != T_FUN_DECL_LIST ){
		printf( "Meh: %s\n", debug_strings[tokens->next->type] );
		move = reduce( tokens->next );
	}

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->down = tokens;
	ret->type = T_FUN_DECL_LIST;
	if ( move->type == T_FUN_DECL_LIST ){
		ret->next = move->next;
		tokens->next = move;
		move->next = NULL;
	} else {
		ret->next = move;
		tokens->next = NULL;
	}
		
	return ret;
}

// Parse rules starting with "id"
parse_node_t *id_stage1( parse_node_t *tokens ){
	printf( "[id_stage1]\n" );
	parse_node_t 	*ret = NULL, 
			*move = NULL;

	if ( !tokens || !tokens->next )
		return tokens; 

	switch ( tokens->next->type ){
		case T_NAME:
			move = id_stage2( tokens );
			break;

		case T_EQUALS:
			move = id_stage3( tokens );
			break;

		case T_OPEN_BRACK:
			move = id_stage4( tokens );
			break;

		case T_OPEN_PAREN:
			move = id_stage5( tokens );
			break;

		default:
			//ret = maptoken( tokens, T_PRIMARY );
			ret = maptoken( tokens, T_EXPR );
			break;
			
	}

	if ( !ret ){
		ret = calloc( 1, sizeof( parse_node_t ));

		if ( move ){
			ret->type = move->status;
			ret->down = tokens;
			ret->next = move->next;
			move->next = NULL;
			move->status = T_NULL;
		} else {
			ret->down = tokens;
			ret->next = tokens->next;
			ret->type = T_PRIMARY;
			tokens->next = NULL;
		}
	}

	return ret;
}

// At this stage, could be: T_VAR_DECL, T_FUN_DECL, T_PARAM_DECL
parse_node_t *id_stage2( parse_node_t *tokens ){
	printf( "[id_stage2]\n" );
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	tokens = tokens->next;

	switch ( tokens->next->type ){
		case T_SEMICOL:
			move = id_stage2_1( tokens );
			break;
		case T_OPEN_PAREN:
			move = id_stage2_2( tokens );
			break;
		default:
			move = id_stage2_3( tokens );
			break;
	}

	if ( move )
		ret = move;

	return ret;
}

// Returns T_VAR_DECL node
parse_node_t *id_stage2_1( parse_node_t *tokens ){
	printf( "[id_stage2_1]\n" );
	parse_node_t *ret;

	if ( !tokens->next )
		return NULL;

	// Makes last node of T_VAR_DECL the semicolon
	ret = tokens->next;
	ret->status = T_VAR_DECL;

	return ret;
}

// T_FUN_DECL, must make sure it's correct.
parse_node_t *id_stage2_2( parse_node_t *tokens ){
	printf( "[id_stage2_2]\n" );
	parse_node_t	*ret = tokens, 
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	tokens = tokens->next;
	printf( "%s\n", debug_strings[tokens->next->type] );
	tokens->next = reduce( tokens->next );

	printf( "\t2_2_1: %s\n", debug_strings[tokens->next->type] );

	switch ( tokens->next->type ){
		case T_PARAM_DECL_LIST:
			printf( "\t[blerg]\n" );
			move = id_stage2_2_1( tokens->next );
			break;

		case T_CLOSE_PAREN:
			move = id_stage2_2_1( tokens );
			break;

		default:
			dump_tree( 0, tokens );
			die( 2, "Expected parameter declaration in function, got %s\n", 
				debug_strings[ tokens->next->type ] );
			break;
	}

	if ( move )
		ret = move;

	return ret;
}

parse_node_t *id_stage2_2_1( parse_node_t *tokens ){
	printf( "[id_stage2_2_1] %s\n", debug_strings[tokens->type] );
	parse_node_t *ret = NULL;

	if ( !tokens->next )
		return NULL;

	tokens = tokens->next;
	tokens->next = reduceto( tokens->next, T_BLOCK );

	if ( tokens->next->type != T_BLOCK ){
		dump_tree( 0, tokens );
		die( 3, "Expected block in function declaration\n" );
	}

	ret = tokens->next;
	ret->status = T_FUN_DECL;

	return ret;
}

// T_PARAM_DECL, make sure it's correct.
parse_node_t *id_stage2_3( parse_node_t *tokens ){
	parse_node_t *ret = tokens;

	ret->status = T_PARAM_DECL;

	return ret;
}

// Parse assignment expression
parse_node_t *id_stage3( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next ) // wut
		return NULL;

	tokens = tokens->next;

	if ( !tokens->next )
		return NULL;

	tokens->next = reduceto( tokens->next, T_EXPR );
	if ( tokens->next->type != T_EXPR ){
		dump_tree( 0, tokens );
		die( 2, "Expected expression for assignment.\n" );
	}

	ret = tokens->next;
	ret->status = T_EXPR;

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

// Parse parameter declaration lists
parse_node_t *paramdecl_stage1( parse_node_t *tokens ){
	printf( "[paramdecl 1] %s\n", debug_strings[tokens->type] );
	parse_node_t 	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	if ( tokens->next->type == T_COMMA ){
		tokens->next->next = move = reduce( tokens->next->next );
		printf( "Blargen: %s\n", debug_strings[move->type] );

	} else if ( tokens->next->type != T_PARAM_DECL_LIST ){
		tokens->next = move = reduce( tokens->next );
		printf( "Blaggen: %s\n", debug_strings[move->type] );

	}

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->down = tokens;
	ret->type = T_PARAM_DECL_LIST;

	if ( move->type == T_PARAM_DECL_LIST ){
		ret->next = move->next;
		tokens->next = move;
		move->next = NULL;

	} else {
		ret->next = move;
		tokens->next = NULL;

	}

	return ret;
}

parse_node_t *return_stage1( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	tokens->next = reduceto( tokens->next, T_EXPR );

	if ( tokens->next->type != T_EXPR ){
		dump_tree( 0, tokens );
		die( 2, "Expected expression after return\n" );
	}

	if ( tokens->next->next->type != T_SEMICOL ){
		dump_tree( 0, tokens );
		die( 2, "Expected semicolon after expression\n" );
	}

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->type = T_STATEMNT;
	ret->next = tokens->next->next->next;
	ret->down = tokens;
	tokens->next->next->next = NULL;

	return ret;
}

parse_node_t *statelist_stage1( parse_node_t *tokens ){
	parse_node_t 	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	if ( tokens->next->type != T_STATEMNT_LIST ){
		printf( "warg: %s\n", debug_strings[tokens->next->type] );
		tokens->next = move = reduce( tokens->next );
	}

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->down = tokens;
	ret->type = T_STATEMNT_LIST;

	if ( move->type == T_STATEMNT_LIST ){
		ret->next = move->next;
		tokens->next = move;
		move->next = NULL;
	} else {
		ret->next = move;
		tokens->next = NULL;
	}
		
	return ret;
}

// Parse variable declaration lists
parse_node_t *vardecl_stage1( parse_node_t *tokens ){
	parse_node_t 	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	if ( tokens->next->type != T_VAR_DECL_LIST ){
		printf( "Hmm: %s\n", debug_strings[tokens->next->type] );
		tokens->next = move = reduce( tokens->next );
	}

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->down = tokens;
	ret->type = T_VAR_DECL_LIST;

	if ( move->type == T_VAR_DECL_LIST ){
		ret->next = move->next;
		tokens->next = move;
		move->next = NULL;
	} else {
		ret->next = move;
		tokens->next = NULL;
	}
		
	return ret;
}

parse_node_t *while_stage1( parse_node_t *tokens ){
	parse_node_t	*ret = NULL,
			*move = NULL;

	if ( !tokens->next )
		return NULL;

	if ( tokens->next->type != T_OPEN_PAREN ){
		dump_tree( 0, tokens );
		die( 2, "Expected opening parenthesis before expression\n" );
	}

	tokens->next->next = reduceto( tokens->next->next, T_EXPR );

	if ( tokens->next->next->type != T_EXPR ){
		dump_tree( 0, tokens );
		die( 2, "Expected expression after parenthesis\n" );
	}

	if ( !tokens->next->next->next )
		return NULL;

	if ( tokens->next->next->next->type != T_CLOSE_PAREN ){
		dump_tree( 0, tokens );
		die( 2, "Expected closing paren after while declaration\n" );
	}

	tokens->next->next->next->next = reduceto( tokens->next->next->next->next, T_STATEMNT );
	if ( tokens->next->next->next->next->type != T_STATEMNT ){
		dump_tree( 0, tokens );
		die( 2, "Expected statement after while declaration\n" );
	}

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->down = tokens;
	ret->next = tokens->next->next->next->next->next;
	tokens->next->next->next->next->next = NULL;
	ret->type = T_STATEMNT;

	return ret;
}





