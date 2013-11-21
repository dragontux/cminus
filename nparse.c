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

// Debugging function, to make sure the rule tables are being generated properly
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

// Generates parsing rules for c--
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
		T_ADD_EXPR, T_SIMPLE_EXPR ), 
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

// Returns the last node in a reduction, with the status set to the returning type
parse_node_t *baseline_iter( parse_node_t *tokens, rule_t *rules ){
	parse_node_t	*ret,
			*move;
	token_type_t	type;
	rule_t		*rmove;

	ret = move = tokens;
	rmove = rules;
	int found = 0;

	if ( move ){
		for ( ; !found && rmove; rmove = rmove->next ){
			printf( "r: \"%s\", ", type_str( rmove->type ));
			if ( rmove->type == move->type ){
				type = rmove->ret;
				printf( "matched. Type set to \"%s\"\n", type_str( type ));

				if ( rmove->down ){
					move->next = reduce( move->next, rmove->down->type );
					ret = baseline_iter( move->next, rmove->down );
					if ( ret->status == T_NULL && type != T_NULL ){
						ret = move;
						ret->status = type;
					}
				} else {
					move->status = type;
					ret = move;
				}

				found = 1;
				break;
			}
		}
	}

	return ret;
}

// Performs one round of reduction
parse_node_t *baseline( parse_node_t *tokens, rule_t *rules ){
	parse_node_t	*ret,
			*move,
			*temp;
	rule_t		*rmove;

	ret = move = tokens;
	rmove = rules;

	move = baseline_iter( move, rules );

	if ( move->status != T_NULL ){
		temp = calloc( 1, sizeof( parse_node_t ));
		temp->type = move->status;
		temp->down = ret;

		temp->next = move->next;
		move->next = NULL;
		move->status = T_NULL;

		ret = temp;
	} 

	if ( ret )
		printf( "returning \"%s\"\n", type_str( ret->type ));

	return ret;
}

// Repeatedly reduces until the returning token is either the topmost expression possible, 
// or until it is of type "type"
parse_node_t *reduce( parse_node_t *tokens, token_type_t type ){
	printf( "-=[ Reducing\n" );
	parse_node_t	*ret = tokens,
			*move = tokens;

	while (( ret = baseline( move, crules )) && ret != move && ret->type != type )
		move = ret;

	printf( "-=[ End reduction\n" );
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
