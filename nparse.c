#include "parse.h"

static rule_t *crules = NULL;
extern char *debug_strings[];

rule_t *add_next( rule_t *node, token_type_t type, token_type_t ret ){
	node->next 		= calloc( 1, sizeof( rule_t ));
	node->next->type	= type;
	node->next->ret		= ret;
	return node->next;
}

rule_t *add_down( rule_t *node, token_type_t type, token_type_t ret ){
	node->down		= calloc( 1, sizeof( rule_t ));
	node->down->type	= type;
	node->down->ret		= ret;
	return node->down;
}

char *type_str( token_type_t type ){
	return debug_strings[ type ];
}

void dump_rules( int level, rule_t *rules ){
	if ( rules ){
		int i;
		for ( i = 0; i < level; i++ )
			printf( "    " );
		
		printf( "%s -> %s\n", type_str( rules->type ), type_str( rules->ret ));
		dump_rules( level+1, rules->down );
		dump_rules( level, rules->next );
	}
}

// Long code, but mostly uninteresting. Generates a tree of rules for use
// by the baseline( ) function.
rule_t *gen_cminus_rules( ){
	rule_t	*ret = NULL,
		*move = NULL,
		*temp = NULL,
		buf;

	move = &buf;

	move = add_next( move,
		T_SIMPLE_EXPR, T_EXPR );

	// var = id | id [ expression ]
	add_down( temp = add_down( move = add_next( move,
		T_NAME, T_VAR ),
			T_EQUALS, T_NULL ),
				T_EXPR, T_EXPR );

	add_down( add_down( add_next( temp,
			T_OPEN_BRACK, T_NULL ),
				T_EXPR, T_NULL ),
					T_CLOSE_BRACK, T_VAR );

	// relop = < | >
	move = add_next( add_next( move,
		T_LESS_THAN, T_REL_OP ),
		T_GREATER_THAN, T_REL_OP );

	// add_expr = add_expr addop term | term
	add_down( temp = add_down( move = add_next( move,
		T_ADD_EXPR, T_NULL/*T_SIMPLE_EXPR*/ ), 
			T_REL_OP, T_NULL ), 
				T_ADD_EXPR, T_SIMPLE_EXPR );

	add_down( add_next( temp,
			T_ADD_OP, T_NULL ), 
				T_TERM, T_ADD_EXPR );
	
	// addop = + | -
	move = add_next( add_next( move,
		T_PLUS,	T_ADD_OP ),
		T_MINUS, T_ADD_OP );
	
	// term = term mulop factor | factor
	add_down( add_down( move = add_next( move,
		T_TERM, T_ADD_EXPR ),
			T_MUL_OP, T_NULL ),
				T_FACTOR, T_TERM );

	move = add_next( move,
		T_FACTOR, T_TERM );

	// mulop = * | /
	move = add_next( add_next( move,
		T_STAR, T_MUL_OP ),
		T_SLASH, T_MUL_OP );

	// factor -> NUM | var
	move = add_next( add_next( move,
		T_INT, T_FACTOR ),
		T_VAR, T_FACTOR );

	ret = buf.next;
	
	return ret;
}

parse_node_t *baseline( parse_node_t *tokens ){
	parse_node_t	*ret,
			*move,
			*temp;
	token_type_t 	type;
	rule_t		*ruleptr,
			*rmove;

	ret = move = tokens;
	rmove = ruleptr = crules;
	type = T_NULL;

	while ( move && rmove ){
		for ( ; rmove; rmove = rmove->next ){
			if ( rmove->type == move->type ){
				type = rmove->ret;

				if ( rmove->down ){
					if ( !move->next )
						die( 2, "Expected token\n" );

					move->next = reduce( move->next, rmove->down->type );
					if ( move->next->type != rmove->down->type )
						continue;

					rmove = rmove->down;
					break;
				} else {
					printf( "No down rule.\n" );
				}
			}
		}

		if ( rmove ){
			move = move->next;
		}
	}

	if ( type != T_NULL ){
		temp = calloc( 1, sizeof( parse_node_t ));
		temp->type = type;
		temp->down = ret;
		temp->next = move->next;

		move->next = NULL;
		ret = temp;
	} 

	printf( "returning \"%s\"\n", type_str( ret->type ));
	return ret;
}

parse_node_t *reduce( parse_node_t *tokens, token_type_t type ){
	parse_node_t	*ret = tokens,
			*move = tokens;

	while (( ret = baseline( move )) && ret != move && ret->type != type )
		move = ret;

	//ret = ret? ret : move;

	/*
	move = NULL;
	while ( ret && ret->type == type && predict( ret ) == type && ret != move ){
		move = ret;
		ret = baseline( ret );
	}
	*/

	return ret;
}

parse_node_t *parse_tokens( parse_node_t *tokens ){
	parse_node_t *ret = NULL;

	if ( !crules )
		crules = gen_cminus_rules( );

	printf( "-=[ Rules dump: \n" );
	dump_rules( 0, crules );
	ret = reduce( tokens, T_PROGRAM );

	return ret;
}

/* // Keep this around in case it's needed in the future
token_type_t predict( parse_node_t *tokens ){
	parse_node_t	*move;
			// *temp;
	token_type_t 	ret,
			type;
	rule_t		*ruleptr,
			*rmove;

	move = tokens;
	rmove = ruleptr = crules;
	type = T_NULL;

	// Iterate through tokens until end of the rule branch
	while ( move && rmove ){
		printf( "-=[predict] move->type: \"%s\"\n", type_str( move->type ));

		// Iterate through current level of rules
		for ( ; rmove; rmove = rmove->next ){
			if ( rmove->type == move->type ){
				printf( "\tMatched type \"%s\"...\n", type_str( rmove->type ));

				type = rmove->ret;
				printf( "\tType set to %s\n", type_str( type ));

				if ( rmove->down ){
					printf( "\tDescended...\n" );
					if ( !move->next )
						break;

					if ( move->next->type != rmove->down->type )
						continue;

					rmove = rmove->down;
					break;
				} else {
					printf( "\tNo down rule.\n" );
				}
			}
		}

		if ( rmove ){
			printf( "\tContinuing to next token\n" );
			move = move->next;
		}
	}

	if ( type == T_NULL )
		type = tokens->type;

	ret = type;

	printf( "\tPredicted \"%s\"\n", type_str( ret ));

	return ret;
}
*/
