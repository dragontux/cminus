#include <cminus/parse.h>

//static rule_t *crules = NULL;
extern char *debug_strings[];

static rule_t *crules =
	&(rule_t){
		.type = T_DECL,
		.ret  = T_DECL_LIST,
		.down = &(rule_t){
			.type = T_DECL_LIST,
			.ret  = T_DECL_LIST,
		},

	.next = &(rule_t){
		// declaration = var-declaration | func-declaration
		.type = T_VAR_DECL,
		.ret  = T_DECL,

	.next = &(rule_t){
		.type = T_FUN_DECL,
		.ret  = T_DECL,

	.next = &(rule_t){
		// param-list = param param-list | param
		.type = T_PARAM_DECL,
		.ret  = T_PARAM_DECL_LIST,

		.down = &(rule_t){
			.type = T_COMMA,

			.down = &(rule_t){
				.type = T_PARAM_DECL_LIST,
				.ret  = T_PARAM_DECL_LIST,
			},
		},
	
	.next = &(rule_t){
		// compound-stmt = { local-decl statement-list } | { statement-list }
		.type = T_OPEN_CURL,

		.down = &(rule_t){
			.type = T_STATEMNT_LIST,

			.down = &(rule_t){
				.type = T_CLOSE_CURL,
				.ret  = T_COMP_STATEMNT,
			},

		.next = &(rule_t){
			.type = T_DECL_LIST,
			.down = &(rule_t){
				.type = T_STATEMNT_LIST,

				.down = &(rule_t){
					.type = T_CLOSE_CURL,
					.ret  = T_COMP_STATEMNT,
				},
			},
		}},
	
	.next = &(rule_t){
		// statement-list = statement statemnt-list | statement
		.type = T_STATEMNT,
		.ret  = T_STATEMNT_LIST,

		.down = &(rule_t){
			.type = T_STATEMNT_LIST,
			.ret  = T_STATEMNT_LIST,
		},

	// statement = expr-stmt | comp-stmt | select-stmt | iter-stmt | return-stmt
	.next = &(rule_t){
		.type = T_EXPR_STATEMNT,
		.ret  = T_STATEMNT,

	.next = &(rule_t){
		.type = T_COMP_STATEMNT,
		.ret  = T_STATEMNT,
	
	.next = &(rule_t){
		.type = T_SELECT_STATEMNT,
		.ret  = T_STATEMNT,
	
	.next = &(rule_t){
		.type = T_ITER_STATEMNT,
		.ret  = T_STATEMNT,

	.next = &(rule_t){
		.type = T_RETURN_STATEMNT,
		.ret  = T_STATEMNT,

	.next = &(rule_t){
		// expression-stmt = expression ; | ;
		.type = T_EXPR,
		.ret  = T_ARGS_LIST,

		.down = &(rule_t){
			.type = T_SEMICOL,
			.ret  = T_EXPR_STATEMNT,

		.next = &(rule_t){
			.type = T_COMMA,

			.down = &(rule_t){
				.type = T_ARGS_LIST,
				.ret  = T_ARGS_LIST,
			},
		}},

	.next = &(rule_t){
		// select-stmt = if ( expression ) statemnt | if ( expression ) statement else statemnt
		.type = T_IF,
		.down = &(rule_t){

			.type = T_OPEN_PAREN,
			.down = &(rule_t){

				.type = T_EXPR,
				.down = &(rule_t){

					.type = T_CLOSE_PAREN,
					.down = &(rule_t){
						
						.type = T_STATEMNT,
						.ret  = T_SELECT_STATEMNT,

						.down = &(rule_t){
							.type = T_ELSE,
							.down = &(rule_t){
								.type = T_STATEMNT,
								.ret  = T_SELECT_STATEMNT,
							},
						},
					},
				},
			},
		},

	.next = &(rule_t){
		// iter-stmt = while ( expression ) statement
		.type = T_WHILE,
		.down = &(rule_t){
			.type = T_OPEN_PAREN,
			.down = &(rule_t){
				.type = T_EXPR,
				.down = &(rule_t){
					.type = T_CLOSE_PAREN,
					.down = &(rule_t){
						.type = T_STATEMNT,
						.ret  = T_ITER_STATEMNT,
					},
				},
			},
		},

	.next = &(rule_t){
		// return-stmt = return ; | return expression ;
		.type = T_RETURN,
		.down = &(rule_t){
			.type = T_EXPR,
			.down = &(rule_t){
				.type = T_SEMICOL,
				.ret  = T_RETURN_STATEMNT,
			},

			.next = &(rule_t){
				.type = T_SEMICOL,
				.ret  = T_RETURN_STATEMNT,
			},
		},

	.next = &(rule_t){
		// expresson = simple_expr
		.type = T_SIMPLE_EXPR,
		.ret  = T_EXPR,

	.next = &(rule_t){
		// var = id | id [ expression ]
		.type = T_NAME,
		.ret  = T_VAR,

		.down = &(rule_t){
			.type = T_NAME,
			.ret  = T_PARAM_DECL,

			.down = &(rule_t){
				.type = T_SEMICOL,
				.ret  = T_VAR_DECL,
				
				.next = &(rule_t){
					.type = T_OPEN_BRACK,
					.down = &(rule_t){
						.type = T_CLOSE_BRACK,
						.ret  = T_PARAM_DECL,
					},

				.next = &(rule_t){
					.type = T_OPEN_PAREN,
					.down = &(rule_t){
						.type = T_PARAM_DECL_LIST,
						.down = &(rule_t){
							.type = T_CLOSE_PAREN,
							.down = &(rule_t){
								.type = T_COMP_STATEMNT,
								.ret  = T_FUN_DECL,

							.next = &(rule_t){
								.type = T_SEMICOL,
								.ret  = T_FUN_DECL,
							}},
						},

					.next = &(rule_t){
						.type = T_CLOSE_PAREN,
						.down = &(rule_t){
							.type = T_COMP_STATEMNT,
							.ret  = T_FUN_DECL,

						.next = &(rule_t){
							.type = T_SEMICOL,
							.ret  = T_FUN_DECL,
						}},
					}},
				}},
			},

			.next = &(rule_t){
				.type = T_OPEN_BRACK,
				.down = &(rule_t){
					.type = T_EXPR,
					.down = &(rule_t){
						.type = T_CLOSE_BRACK,
						.ret  = T_VAR,
					},
				},

			.next = &(rule_t){
				.type = T_OPEN_PAREN,

				.down = &(rule_t){
					.type = T_CLOSE_PAREN,
					.ret  = T_CALL,

				.next = &(rule_t){
					.type = T_ARGS_LIST,
					.down = &(rule_t){
						.type = T_CLOSE_PAREN,
						.ret  = T_CALL,
					},
				}},

			}},
		},

	.next = &(rule_t){
		// relop = < | >
		.type = T_LESS_THAN,
		.ret  = T_REL_OP,

	.next = &(rule_t){
		.type = T_GREATER_THAN,
		.ret  = T_REL_OP,

	.next = &(rule_t){
		// add_expr = add_expr addop term | term
		.type = T_ADD_EXPR,
		.ret  = T_SIMPLE_EXPR,

		.down = &(rule_t){
			.type = T_REL_OP,
			.down = &(rule_t){
				.type = T_ADD_EXPR,
				.ret  = T_SIMPLE_EXPR,
			},

		.next = &(rule_t){
			.type = T_ADD_OP,
			.down = &(rule_t){
				.type = T_TERM,
				.ret  = T_ADD_EXPR,
			},
		}},

	// addop = + | -
	.next = &(rule_t){
		.type = T_PLUS,
		.ret  = T_ADD_OP,

	.next = &(rule_t){
		.type = T_MINUS,
		.ret  = T_ADD_OP,

	.next = &(rule_t){
		.type = T_TERM,
		.ret  = T_ADD_EXPR,

	.next = &(rule_t){
		.type = T_FACTOR,
		.ret  = T_TERM,

		.down = &(rule_t){
			.type = T_MUL_OP,
			.down = &(rule_t){
				.type = T_TERM,
				.ret  = T_TERM,
			},
		},

	// mulop = * | /
	.next = &(rule_t){
		.type = T_STAR,
		.ret  = T_MUL_OP,

	.next = &(rule_t){
		.type = T_SLASH,
		.ret  = T_MUL_OP,

	.next = &(rule_t){
		.type = T_VAR,
		.ret  = T_FACTOR,

		.down = &(rule_t){
			.type = T_EQUALS,
			.down = &(rule_t){
				.type = T_EXPR,
				.ret  = T_EXPR,
			},
		},

	// factor -> NUM | var | call | string
	.next = &(rule_t){
		.type = T_INT,
		.ret  = T_FACTOR,

	.next = &(rule_t){
		.type = T_CALL,
		.ret  = T_FACTOR,

	.next = &(rule_t){
		.type = T_STRING,
		.ret  = T_FACTOR,

	}}}}}}}}}}}}}}}}}}}}}}}}}}}}}};

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
			//printf( "r: \"%s\", ", type_str( rmove->type ));
			if ( rmove->type == move->type ){
				type = rmove->ret;
				//printf( "matched. Type set to \"%s\"\n", type_str( type ));

				if ( rmove->down ){
					if ( move->next->type != rmove->down->type )
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

	/*
	if ( ret )
		printf( "returning \"%s\"\n", type_str( ret->type ));
	*/

	return ret;
}

int has_higher_prec( token_type_t top, token_type_t bottom, rule_t *rules ){
	rule_t *ruleptr = rules;
	int ret, found;

	for ( ret = found = 0; !found && ruleptr->next; ruleptr = ruleptr->next ){
		if ( ruleptr->type == top ){
			ret = 1;
			found = 1;
		} else if ( ruleptr->type == bottom ){
			ret = 0;
			found = 1;
		}
	}

	return ret;
}

// Repeatedly reduces until the returning token is either the topmost expression possible, 
// or until it is of type "type"
parse_node_t *reduce( parse_node_t *tokens, token_type_t type ){
	parse_node_t	*ret = tokens,
			*move = tokens;

	while (( ret = baseline( move, crules )) && ret != move && ret->type != type )
		move = ret;

	return ret;
}

parse_node_t *parse_tokens( parse_node_t *tokens ){
	parse_node_t *ret = NULL;

	/*
	if ( !crules )
		crules = gen_cminus_rules( );
		*/

	//printf( "-=[ Rules dump: \n" );
	//dump_rules( 0, crules );
	ret = reduce( tokens, T_PROGRAM );

	return ret;
}

// TODO: these functions are deprecated, remove them if no bugs show up as a result of the rewrite
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


// Generates parsing rules for c--
rule_t *gen_cminus_rules( ){
	rule_t	*ret = NULL,
		*move = NULL,
		*temp = NULL,
		*blarg = NULL,
		buf;

	move = &buf;

	/*
	// program = decl-list
	move = add_next( move, 
		T_DECL_LIST, T_PROGRAM );
	*/

	// decl-list = declaration decl-list | declaration
	add_down( move = add_next( move,
		T_DECL, T_DECL_LIST ),
			T_DECL_LIST, T_DECL_LIST );

	// declaration = var-declaration | func-declaration
	move = add_next( move, 
		T_VAR_DECL, T_DECL );

	move = add_next( move,
		T_FUN_DECL, T_DECL );

	// param-list = param param-list | param
	add_down( add_down( move = add_next( move,
		T_PARAM_DECL, T_PARAM_DECL_LIST ),
			T_COMMA, T_NULL ),
				T_PARAM_DECL_LIST, T_PARAM_DECL_LIST );

	// compound-stmt = { local-decl statement-list } | { statement-list }
	add_down( temp = add_down( move = add_next( move,
		T_OPEN_CURL, T_NULL ),
			T_STATEMNT_LIST, T_NULL ),
				T_CLOSE_CURL, T_COMP_STATEMNT );

	add_down( add_down( add_next( temp,
			T_DECL_LIST, T_NULL ),
				T_STATEMNT_LIST, T_NULL ),
					T_CLOSE_CURL, T_COMP_STATEMNT );

	// statement-list = statement statemnt-list | statement
	add_down( move = add_next( move,
		T_STATEMNT, T_STATEMNT_LIST ),
			T_STATEMNT_LIST, T_STATEMNT_LIST );

	// statement = expr-stmt | comp-stmt | select-stmt | iter-stmt | return-stmt
	move = add_next( move,
		T_EXPR_STATEMNT, T_STATEMNT );

	move = add_next( move,
		T_COMP_STATEMNT, T_STATEMNT );

	move = add_next( move,
		T_SELECT_STATEMNT, T_STATEMNT );

	move = add_next( move,
		T_ITER_STATEMNT, T_STATEMNT );

	move = add_next( move,
		T_RETURN_STATEMNT, T_STATEMNT );

	// expression-stmt = expression ; | ;
	temp = add_down( move = add_next( move,
		T_EXPR,	T_ARGS_LIST ),
			T_SEMICOL, T_EXPR_STATEMNT );
	
	add_down( add_next( temp,
			T_COMMA, T_NULL ),
				T_ARGS_LIST, T_ARGS_LIST );

	/*
	move = add_next( move, 
		T_SEMICOL, T_EXPR_STATEMNT );
	*/

	// select-stmt = if ( expression ) statemnt | if ( expression ) statement else statemnt
	add_down( add_down( add_down( add_down( add_down( add_down( move = add_next( move,
		T_IF, T_NULL ),
			T_OPEN_PAREN, T_NULL ),
				T_EXPR, T_NULL ),
					T_CLOSE_PAREN, T_NULL ),
						T_STATEMNT, T_SELECT_STATEMNT ),
							T_ELSE, T_NULL ),
								T_STATEMNT, T_SELECT_STATEMNT );

	// iter-stmt = while ( expression ) statement
	add_down( add_down( add_down( add_down( move = add_next( move,
		T_WHILE, T_NULL ),
			T_OPEN_PAREN, T_NULL ),
				T_EXPR, T_NULL ),
					T_CLOSE_PAREN, T_NULL ),
						T_STATEMNT, T_ITER_STATEMNT );

	// return-stmt = return ; | return expression ;
	add_down( temp = add_down( move = add_next( move,
		T_RETURN, T_NULL ),
			T_EXPR, T_NULL ),
				T_SEMICOL, T_RETURN_STATEMNT );

	add_next( temp,
			T_SEMICOL, T_RETURN_STATEMNT );


	// expresson = simple_expr
	move = add_next( move,
		T_SIMPLE_EXPR, T_EXPR );

	// var = id | id [ expression ]
	blarg = add_down( temp = add_down( move = add_next( move,
		// Handle variable declarations
		T_NAME, T_VAR ),
			T_NAME, T_PARAM_DECL ),
				T_SEMICOL, T_VAR_DECL );

	add_down( blarg = add_next( blarg,
				T_OPEN_BRACK, T_NULL ),
					T_CLOSE_BRACK, T_PARAM_DECL );

	// handle function declarations
	add_down( add_down( blarg = add_down( add_next( blarg,
				T_OPEN_PAREN, T_NULL ),
					T_PARAM_DECL_LIST, T_NULL ),
						T_CLOSE_PAREN, T_NULL ),
							T_COMP_STATEMNT, T_FUN_DECL );

	add_down( add_next( blarg,
					T_CLOSE_PAREN, T_NULL ),
						T_COMP_STATEMNT, T_FUN_DECL );

	add_down( add_down( temp = add_next( temp,
			T_OPEN_BRACK, T_NULL ),
				T_EXPR, T_NULL ),
					T_CLOSE_BRACK, T_VAR );

	add_down( temp = add_down( add_next( temp,
			T_OPEN_PAREN, T_NULL ),
				T_ARGS_LIST, T_NULL ),
					T_CLOSE_PAREN, T_CALL );

	add_next( temp,
				T_CLOSE_PAREN, T_CALL );


	// relop = < | >
	move = add_next( move,
		T_LESS_THAN, T_REL_OP );
	move = add_next( move,
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
	
	move = add_next( move,
		T_TERM, T_ADD_EXPR );

	add_down( add_down( move = add_next( move,
		T_FACTOR, T_TERM ),
			T_MUL_OP, T_NULL ),
				T_TERM, T_TERM );
			

	// mulop = * | /
	move = add_next( add_next( move,
		T_STAR, T_MUL_OP ),
		T_SLASH, T_MUL_OP );

	add_down( add_down( move = add_next( move,
		T_VAR, T_FACTOR ),
			T_EQUALS, T_NULL ),
				T_EXPR, T_EXPR );

	// factor -> NUM | var | call | string
	move = add_next( add_next( add_next( move,
		T_INT, T_FACTOR ),
		T_CALL, T_FACTOR ),
		T_STRING, T_FACTOR );

	/*
	add_down( add_down( move = add_next( move,
		T_OPEN_PAREN, T_NULL ),
			T_EXPR, T_NULL ),
				T_CLOSE_PAREN, T_FACTOR );
	*/

	ret = buf.next;
	
	return ret;
}
