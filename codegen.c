#include "parse.h"
#include "codegen.h"
#include <string.h>

extern char *type_str( token_type_t );

parse_node_t *clone_tree( parse_node_t *tokens ){
	parse_node_t *ret = NULL;

	if ( tokens ){
		ret = malloc( sizeof( *tokens ));
		*ret = *tokens;

		ret->down = clone_tree( ret->down );
		ret->next = clone_tree( ret->next );

		if ( ret->data )
			ret->data = strdup( ret->data );
	}

	return ret;
}

parse_node_t *trim_tree( parse_node_t *tree ){
	parse_node_t 	*ret = NULL;

	if ( tree ){
		ret = tree;
		if (( ret->next == NULL || tree->next->type == T_NULL ) &&
				( tree->down != NULL && !tree->down->next )){

			ret = trim_tree( tree->down );
			free( tree );

		} else if ( tree->down != NULL && !tree->down->next && tree->type != T_STATEMNT ){
			ret = trim_tree( tree->down );
			ret->next = trim_tree( tree->next );
			free( tree );

		} else {
			ret->down = trim_tree( tree->down );
			ret->next = trim_tree( tree->next );

		}
	}

	return ret;
}

parse_node_t *strip_token( parse_node_t *tree, token_type_t type ){
	parse_node_t *ret = tree;

	if ( tree ){
		if ( tree->type == type ){
			ret = strip_token( tree->next, type );
			free( tree );
		} else {
			ret->down = strip_token( tree->down, type );
			ret->next = strip_token( tree->next, type );
		}
	}

	return ret;
}

tac_var_t *new_tac_var( struct tac *val, parse_node_t *node ){
	tac_var_t *ret = calloc( 1, sizeof( tac_var_t ));

	ret->temp = val;
	ret->node = node;
	ret->is_node = !!node;

	return ret;
}

tac_t *ptot_iter( parse_node_t *node, tac_t *list ){
	tac_t *ret = NULL;

	return ret;
}

enum {
	STATE_IN_FUNCTION = 1,
};

enum {
	VAR_PLACE_GLOBAL,
	VAR_PLACE_PARAMETER,
	VAR_PLACE_LOCAL,
};

struct gen_state;

typedef struct name_decl {
	char *name;
	char *type;
	unsigned placement;
	unsigned number;
	struct gen_state *state;
	int reg;

	struct name_decl *next;
} name_decl_t;

typedef struct gen_state {
	unsigned flags;
	unsigned num_params;
	unsigned num_vars;
	struct gen_state *last;
	struct name_decl *namelist;
} gen_state_t;

name_decl_t *find_name_no_recurse( char *name, gen_state_t *state ){
	name_decl_t *move;
	name_decl_t *ret = NULL;

	if ( state ){
		move = state->namelist;

		while ( move ){
			if ( strcmp( move->name, name ) == 0 ){
				ret = move;
				break;
			}

			move = move->next;
		}
	}

	return ret;
}

name_decl_t *find_name( char *name, gen_state_t *state ){
	name_decl_t *move;
	name_decl_t *ret = NULL;
	gen_state_t *cur_state = state;

	while ( cur_state && !ret ){
		ret = find_name_no_recurse( name, cur_state );

		if ( ret ){
			break;

		} else {
			cur_state = cur_state->last;
		}
	}

	return ret;
}


void add_name( gen_state_t *state, name_decl_t *new_name ){
	if ( new_name && state ){
		new_name->next = state->namelist;
		state->namelist = new_name;
	}
}

void free_namelist( name_decl_t *namelist ){
	if ( namelist ){
		free_namelist( namelist->next );
		free( namelist );
	}
}

unsigned blarg( parse_node_t *tree, unsigned address, gen_state_t *state ){
	if ( tree ){
		switch( tree->type ){
			case T_DECL_LIST:
				address = blarg( tree->down, address + 1, state );
				address = blarg( tree->next, address + 1, state );
				break;

			case T_VAR_DECL:
				{
					name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));
					name_decl_t *move;

					move = find_name_no_recurse( tree->down->next->data, state );

					if ( move ){
						printf( "Error: symbol \"%s\" already defined in this scope\n", tree->down->next->data );
					}

					printf( "%4d > ", address );

					state->num_vars++;

					if ( state->flags & STATE_IN_FUNCTION ){
						printf( "    sub rsp, 8 ; %s %s\n",
								tree->down->data, tree->down->next->data );

						new_name->type = tree->down->data;
						new_name->name = tree->down->next->data;
						new_name->placement = VAR_PLACE_LOCAL;
						new_name->number = state->num_vars;
						new_name->state = state;

					} else {
						printf( ".section bss\n" );

						printf( "%4d > ", address );
						printf( "global %s\n", tree->down->next->data );

						printf( "%4d > ", address );
						printf( "%s: dq 0 ; %s\n",
								tree->down->next->data, tree->down->data );

						new_name->type = tree->down->data;
						new_name->name = tree->down->next->data;
						new_name->placement = VAR_PLACE_GLOBAL;
						new_name->number = state->num_vars;
						new_name->state = state;
					}

					add_name( state, new_name );

					address = blarg( tree->next, address + 1, state );
				}

				break;

			case T_FUN_DECL:
				{
					gen_state_t newscope = {
						.last = state,
						.num_params = 0,
						.flags = STATE_IN_FUNCTION,
					};

					name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));
					name_decl_t *move;

					move = find_name( tree->down->next->data, state );

					if ( !move ){
						new_name->type = tree->down->data;
						new_name->name = tree->down->next->data;
						new_name->placement = VAR_PLACE_GLOBAL;
						new_name->number = state->num_vars;
						new_name->state = state;

						add_name( state, new_name );

					} else {
						printf( "Error: symbol \"%s\" redefined\n", tree->down->next->data );
					}

					printf( "%4d*> ", address );
					printf( ".section text\n" );

					printf( "%4d*> ", address );
					printf( "global %s\n", tree->down->next->data );

					printf( "%4d*> ", address );
					printf( "%s: ; returns %s\n",
							tree->down->next->data, tree->down->data );

					printf( "%4d*> ", address );
					printf( "    push rbp\n" );
					printf( "%4d*> ", address );
					printf( "    mov rbp, rsp\n" );

					address = blarg( tree->down->next->next, address + 1, &newscope ) + 1;

					printf( "%4d > ", address );
					printf( "    mov rsp, rbp\n" );

					printf( "%4d > ", address );
					printf( "    pop rbp\n" );

					printf( "%4d > ", address );
					printf( "    ret\n" );

					address = blarg( tree->next, address + 1, state );
				}

				break;

			case T_PARAM_DECL_LIST:
				address = blarg( tree->down, address + 1, state );
				address = blarg( tree->next, address + 1, state );
				break;

			case T_PARAM_DECL:
				state->num_params++;

				name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));
				new_name->type = tree->down->data;
				new_name->name = tree->down->next->data;
				new_name->placement = VAR_PLACE_PARAMETER;
				new_name->number = state->num_params;
				new_name->state = state;
				add_name( state, new_name );

				printf( "%4d > ", address );
				printf( "    ; parameter \"%s\" of type \"%s\" at [rbp-%u]\n",
						tree->down->next->data, tree->down->data, state->num_params * 8 );

				address = blarg( tree->next, address + 1, state );// + 1;
				break;

			case T_STATEMNT_LIST:
			case T_COMP_STATEMNT:
			case T_STATEMNT:
				address = blarg( tree->down, address + 1, state );
				address = blarg( tree->next, address + 1, state );
				break;

			case T_ITER_STATEMNT:
				{
					unsigned test_start, test_addr;

					test_start = address;
					printf( "%4d > ", address );
					printf( "    ; iterator statement, start at %u\n", test_start );

					printf( "%4d > ", address );
					printf( ".while_%u:\n", test_start );

					test_addr = blarg( tree->down->next, address + 1, state );
					address = test_addr + 1;

					printf( "%4d > ", address );
					printf( "    test rax, rax ; check result from %u\n", test_addr );

					printf( "%4d > ", address );
					printf( "    jz .end_while_%u\n", test_start );

					printf( "%4d > ", address );
					printf( "    ; iterator statement \"%s\", test at %u. If test is 0 jump to end\n",
							type_str( tree->down->type ), test_addr );

					address = blarg( tree->down->next->next, address + 1, state );
					address++;

					printf( "%4d > ", address );
					printf( "    jmp .while_%u\n", test_start );

					printf( "%4d > ", address );
					printf( "    ; end of iterator at %u\n", test_start );

					printf( "%4d > ", address );
					printf( ".end_while_%u:\n", test_start );

					//address = blarg( tree->next, address + 1 ) + 1;
					address = blarg( tree->next, address + 1, state );
				}

				break;

			case T_EXPR:
			case T_ADD_EXPR:
			case T_TERM:
			case T_SIMPLE_EXPR:
				{
					unsigned op1, op2;

					printf( "%4d > ", address );
					printf( "    ; Have expression, operation is %s\n", type_str( tree->down->next->type ));
					op1 = blarg( tree->down, address + 1, state );
					address = op1 + 1;;

					op2 = blarg( tree->down->next->next, address + 1, state );
					address = op2 + 1;;

					printf( "%4d > ", address );
					printf( "    ; End of expression, ops are %u and %u\n", op1, op2 );
				}

				break;

			case T_SELECT_STATEMNT:
				{
					unsigned test_addr, start, body_addr, else_addr = 0, end;

					test_addr = blarg( tree->down->next, address + 1, state );
					address = test_addr + 1;

					start = address;

					printf( "%4d > ", address );
					printf( ".if_%u:\n", start );

					printf( "%4d > ", address );
					printf( "    test rax, rax ; check result from %u\n", test_addr );

					printf( "%4d > ", address );
					printf( "    jnz .else_if_%u\n", start );

					body_addr = blarg( tree->down->next->next, address + 1, state );
					address = body_addr + 1;

					printf( "%4d > ", address );
					printf( "    jmp .end_if_%u\n", start );

					printf( "%4d > ", address );
					printf( ".else_if_%u:\n", start );

					if ( tree->down->next->next->next ){
						else_addr = blarg( tree->down->next->next->next->next, address + 1, state );
						address = else_addr + 1;
					}

					printf( "%4d > ", address );
					printf( ".end_if_%u:\n", start );

					address = blarg( tree->next, address + 1, state );
				}

				break;

			case T_RETURN_STATEMNT:
				{
					unsigned return_addr;

					return_addr = blarg( tree->down->next, address + 1, state );
					address = return_addr + 1;

					printf( "%4d > ", address );
					printf( "    ; Have return statement, return value at %u\n", return_addr );

					address = blarg( tree->next, address + 1, state );
				}
				break;

			case T_VAR:
				printf( "%4d > ", address );
				printf( "    ; Have variable \"%s\", index %d\n",
						tree->down->data, *(int *)tree->down->next->next->data );
				
				break;

			case T_CALL:
				{
					parse_node_t *move;
					unsigned param;

					for ( move = tree->down->next; move; move = move->next ){
						param = blarg( move, address + 1, state );
						address = param + 1;

						printf( "%4d > ", address );
						printf( "    push rax ; store %d as parameter\n", param );

						address++;
					}

					printf( "%4d > ", address );
					printf( "    call %s\n", tree->down->data );
				}

				break;

			case T_INT:
			case T_STRING:
			case T_CHAR:
			case T_DOUBLE:
				printf( "%4d > ", address );
				printf( "    ; Have basic datatype \"%s\"\n", type_str( tree->type ));
				break;

			case T_NAME:
				{
					name_decl_t *name = find_name( tree->data, state );

					if ( name ){
						printf( "%4d > ", address );
						printf( "    ; Have name, \"%s\", at %p and with placement %u\n",
								tree->data, name, name->placement );

						switch( name->placement ){
							case VAR_PLACE_GLOBAL:
								printf( "%4d > ", address );
								printf( "    mov rax, [%s]\n", tree->data );
								break;

							case VAR_PLACE_PARAMETER:
								printf( "%4d > ", address );
								printf( "    mov rax, [rbp-%u]\n", name->number * 8 );
								break;

							case VAR_PLACE_LOCAL:
								printf( "%4d > ", address );
								printf( "    mov rax, [rsp+%u]\n", name->number * 8 );
								break;

							default:
								break;
						}

					} else {
						printf( "Error: symbol \"%s\" undefined\n", tree->data );
					}
				}

				break;

			default:
				printf( "%4d > ", address );
				printf( "    ; Skipping type \"%s\"...\n", type_str( tree->type ));
				break;
		}
	}

	return address;
}

tac_t *ptree_to_tlist( parse_node_t *tree ){
	tac_t *ret = NULL;
	parse_node_t *wut = clone_tree( tree );
	gen_state_t state;

	printf( "-=[ Original:\n" );
	dump_tree( 0, tree );

	strip_token( wut, T_SEMICOL );
	strip_token( wut, T_COMMA );
	strip_token( wut, T_OPEN_CURL );
	strip_token( wut, T_CLOSE_CURL );
	strip_token( wut, T_OPEN_PAREN );
	strip_token( wut, T_CLOSE_PAREN );

	printf( "-=[ New: \n" );
	dump_tree( 0, wut );

	printf( "-=[ Reduced: \n" );
	wut = trim_tree( wut );
	dump_tree( 0, wut );

	putchar( '\n' );

	memset( &state, 0, sizeof( state ));
	blarg( wut, 0, &state );

	return ret;
}
