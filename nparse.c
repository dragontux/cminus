#include "parse.h"

static rule_t *crules = NULL;
extern char *debug_strings[];

rule_t *gen_cminus_rules( ){
	rule_t	*ret = NULL,
		*move = NULL,
		*temp = NULL;

	move = ret = calloc( 1, sizeof( rule_t ));
	move->type = T_FACTOR;
	move->ret = T_TERM;

	move = move->next = calloc( 1, sizeof( rule_t ));
	move->type = T_TERM;

	temp = move->down = calloc( 1, sizeof( rule_t ));
	temp->type = T_MUL_OP;

	temp = temp->down = calloc( 1, sizeof( rule_t ));
	temp->type = T_FACTOR;
	temp->ret = T_TERM;

	move = move->next = calloc( 1, sizeof( rule_t ));
	move->type = T_INT;
	move->ret = T_FACTOR;

	move = move->next = calloc( 1, sizeof( rule_t ));
	move->type = T_STAR;
	move->ret = T_MUL_OP;
	
	return ret;
}

char *type_str( token_type_t type ){
	return debug_strings[ type ];
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

	// Iterate through tokens until end of the rule branch
	while ( move && rmove ){
		printf( "-=[baseline] move->type: \"%s\"\n", type_str( move->type ));

		// Iterate through current level of rules
		for ( ; rmove; rmove = rmove->next ){
			if ( rmove->type == move->type ){
				printf( "Matched type \"%s\"...\n", type_str( rmove->type ));

				type = rmove->ret;
				printf( "Type set to %s\n", type_str( type ));

				if ( rmove->down ){
					printf( "Descended...\n" );
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
			printf( "Continuing to next token\n" );
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

	return ret;
}

/*
token_type_t predict( parse_node_t *tokens ){
	type_t	ret,
		move;

	ret = move = T_NULL;

	return ret;
}
*/

parse_node_t *reduce( parse_node_t *tokens, token_type_t type ){
	parse_node_t	*ret = tokens,
			*move = tokens;

	while (( ret = baseline( move )) && ret != move && ret->type != type )
		move = ret;

	/*
	ret = ret? ret : move;

	while ( ret && ret->type == type && predict( ret ) == type && ret != move ){
		move = ret;
		ret = reduce( );
	}
	*/

	return ret;
}

parse_node_t *parse_tokens( parse_node_t *tokens ){
	parse_node_t *ret = NULL;

	if ( !crules )
		crules = gen_cminus_rules( );

	ret = reduce( tokens, T_PROGRAM );

	return ret;
}

