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

unsigned blarg( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp );

unsigned handle_decl_list( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	address = blarg( tree->down, address + 1, state, fp );
	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_var_decl( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
	name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));
	name_decl_t *move;

	move = find_name_no_recurse( tree->down->next->data, state );

	if ( move ){
		fprintf( stderr, "Error: symbol \"%s\" already defined in this scope\n", tree->down->next->data );
	}

//	fprintf( fp, "    ;%4d > \n", address );

	state->num_vars++;

	if ( state->flags & STATE_IN_FUNCTION ){
		fprintf( fp, "    sub rsp, 8 ; %s %s\n",
				tree->down->data, tree->down->next->data );

		new_name->type = tree->down->data;
		new_name->name = tree->down->next->data;
		new_name->placement = VAR_PLACE_LOCAL;
		new_name->number = state->num_vars;
		new_name->state = state;

	} else {
		fprintf( fp, "section .bss\n" );

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "global %s\n", tree->down->next->data );

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "%s: resq 1 ; %s\n",
				tree->down->next->data, tree->down->data );

		new_name->type = tree->down->data;
		new_name->name = tree->down->next->data;
		new_name->placement = VAR_PLACE_GLOBAL;
		new_name->number = state->num_vars;
		new_name->state = state;
	}

	add_name( state, new_name );

	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_func_decl( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
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
		fprintf( stderr, "Error: symbol \"%s\" redefined\n", tree->down->next->data );
	}

//	fprintf( fp, "    ;%4d*> \n", address );
	fprintf( fp, "section .text\n" );

//	fprintf( fp, "    ;%4d*> \n", address );
	fprintf( fp, "global %s\n", tree->down->next->data );

//	fprintf( fp, "    ;%4d*> \n", address );
	fprintf( fp, "%s: ; returns %s\n",
			tree->down->next->data, tree->down->data );

//	fprintf( fp, "    ;%4d*> \n", address );
	fprintf( fp, "    push rbp\n" );
//	fprintf( fp, "    ;%4d*> \n", address );
	fprintf( fp, "    mov rbp, rsp\n" );

	address = blarg( tree->down->next->next, address + 1, &newscope, fp ) + 1;

//	fprintf( fp, "    ;%4d*> \n", address );
	fprintf( fp, ".function_end:\n" );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    mov rsp, rbp\n" );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    pop rbp\n" );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ret\n" );

	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_param_decl_list( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
	address = blarg( tree->down, address + 1, state, fp );
	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_param_decl( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
	state->num_params++;

	name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));
	new_name->type = tree->down->data;
	new_name->name = tree->down->next->data;
	new_name->placement = VAR_PLACE_PARAMETER;
	new_name->number = state->num_params;
	new_name->state = state;
	add_name( state, new_name );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; parameter \"%s\" of type \"%s\" at [rbp-%u]\n",
	tree->down->next->data, tree->down->data, state->num_params * 8 );

	address = blarg( tree->next, address + 1, state, fp );// + 1;

	return address;
}

unsigned handle_statement( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	address = blarg( tree->down, address + 1, state, fp );
	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_iter_statement( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned test_start, test_addr;

	test_start = address;
//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; iterator statement, start at %u\n", test_start );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, ".while_%u:\n", test_start );

	test_addr = blarg( tree->down->next, address + 1, state, fp );
	address = test_addr + 1;

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    test rax, rax ; check result from %u\n", test_addr );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    jz .end_while_%u\n", test_start );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; iterator statement \"%s\", test at %u. If test is 0 jump to end\n",
	type_str( tree->down->type ), test_addr );

	address = blarg( tree->down->next->next, address + 1, state, fp );
	address++;

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    jmp .while_%u\n", test_start );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; end of iterator at %u\n", test_start );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, ".end_while_%u:\n", test_start );

	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_assignment_expression(
		parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp )
{
	unsigned op;

	// TODO: Make sure that tree->down->data is actually a name symbol,
	//       just in case
	name_decl_t *name = find_name( tree->down->data, state );

	if ( name ){
		op = blarg( tree->down->next->next, address + 1, state, fp );
		address = op + 1;

		switch( name->placement ){
			case VAR_PLACE_GLOBAL:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov [%s], rax\n", tree->down->data );
				break;

			case VAR_PLACE_PARAMETER:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov [rbp+%u], rax\n", (name->number + 1) * 8 );
				break;

			case VAR_PLACE_LOCAL:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov [rbp-%u], rax\n", name->number * 8 );
				break;

			default:
				break;
		}

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    ; End of expression, op are %u\n", op );
	}

	return address;
}

unsigned handle_expression( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned op1, op2;

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; Have expression, operation is %s\n", type_str( tree->down->next->type ));

	if ( tree->down->next->type == T_EQUALS ){
		address = handle_assignment_expression( tree, address + 1, state, fp );
		
	} else {
		op1 = blarg( tree->down, address + 1, state, fp );
		address = op1 + 1;

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    mov rbx, rax\n" );

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    push rbx\n" );

		op2 = blarg( tree->down->next->next, address + 1, state, fp );
		address = op2 + 1;

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    pop rbx\n" );

		switch( tree->down->next->type ){
			case T_PLUS:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    add rax, rbx\n" );
				address++;
				break;

			case T_MINUS:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    sub rbx, rax\n" );
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rax, rbx\n" );
				address++;
				break;

			case T_STAR:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mul rbx\n" );
				address++;
				break;

			case T_SLASH:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rcx, rax\n" );

//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rax, rbx\n" );

//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rdx, 0\n" );

//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    div rcx\n" );
				address++;
				break;

			default:
				break;
		}

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    ; End of expression, ops are %u and %u\n", op1, op2 );
	}

	return address;
}

unsigned handle_select( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned test_addr, start, body_addr, else_addr = 0, end;

	test_addr = blarg( tree->down->next, address + 1, state, fp );
	address = test_addr + 1;

	start = address;

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, ".if_%u:\n", start );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    test rax, rax ; check result from %u\n", test_addr );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    jz .else_if_%u\n", start );

	body_addr = blarg( tree->down->next->next, address + 1, state, fp );
	address = body_addr + 1;

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    jmp .end_if_%u\n", start );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, ".else_if_%u:\n", start );

	if ( tree->down->next->next->next ){
		else_addr = blarg( tree->down->next->next->next->next, address + 1, state, fp );
		address = else_addr + 1;
	}

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, ".end_if_%u:\n", start );

	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_return( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned return_addr;

	return_addr = blarg( tree->down->next, address + 1, state, fp );
	address = return_addr + 1;

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; Have return statement, return value at %u\n", return_addr );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    jmp .function_end\n" );

	address = blarg( tree->next, address + 1, state, fp );

	return address;
}

unsigned handle_variable( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; Have variable \"%s\", index %d\n",
			tree->down->data, *(int *)tree->down->next->next->data );

	return address;
}

unsigned handle_call( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	parse_node_t *move;
	unsigned param;
	unsigned i;

	for ( move = tree->down->next, i = 0; move; move = move->next, i++ ){
		if ( move->type == T_ARGS_LIST )
			move = move->down;

		param = blarg( move, address + 1, state, fp );
		address = param + 1;

//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    push rax ; store %d as parameter %u\n", param, i );

		address++;
	}

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    call %s\n", tree->down->data );

//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    add rsp, %u ; restore stack\n", i * 8 );

	return address;
}

unsigned handle_name( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	name_decl_t *name = find_name( tree->data, state );

	if ( name ){
//		fprintf( fp, "    ;%4d > \n", address );
		fprintf( fp, "    ; Have name, \"%s\", at %p and with placement %u\n",
				tree->data, name, name->placement );

		switch( name->placement ){
			case VAR_PLACE_GLOBAL:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rax, [%s]\n", tree->data );
				break;

			case VAR_PLACE_PARAMETER:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rax, [rbp+%u]\n", (name->number + 1) * 8 );
				break;

			case VAR_PLACE_LOCAL:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    mov rax, [rbp-%u]\n", name->number * 8 );
				break;

			default:
				break;
		}

	} else {
		fprintf( stderr, "Error: symbol \"%s\" undefined\n", tree->data );
	}

	return address;
}

unsigned handle_int( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; Have basic datatype \"%s\"\n", type_str( tree->type ));
//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    mov rax, %u\n", *(int *)tree->data );

	return address;
}

unsigned handle_basic_datatype( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
//	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; Have basic datatype \"%s\"\n", type_str( tree->type ));

	return address;
}

unsigned blarg( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	if ( tree ){
		switch( tree->type ){
			case T_DECL_LIST:
				address = handle_decl_list( tree, address, state, fp );
				break;

			case T_VAR_DECL:
				address = handle_var_decl( tree, address, state, fp );
				break;

			case T_FUN_DECL:
				address = handle_func_decl( tree, address, state, fp );
				break;

			case T_PARAM_DECL_LIST:
				address = handle_param_decl_list( tree, address, state, fp );
				break;

			case T_PARAM_DECL:
				address = handle_param_decl( tree, address, state, fp );
				break;

			case T_STATEMNT_LIST:
			case T_COMP_STATEMNT:
			case T_STATEMNT:
				address = handle_statement( tree, address, state, fp );
				break;

			case T_ITER_STATEMNT:
				address = handle_iter_statement( tree, address, state, fp );
				break;

			case T_EXPR:
			case T_ADD_EXPR:
			case T_TERM:
			case T_SIMPLE_EXPR:
				address = handle_expression( tree, address, state, fp );
				break;

			case T_SELECT_STATEMNT:
				address = handle_select( tree, address, state, fp );
				break;

			case T_RETURN_STATEMNT:
				address = handle_return( tree, address, state, fp );
				break;

			case T_VAR:
				address = handle_variable( tree, address, state, fp );
				break; 

			case T_CALL:
				address = handle_call( tree, address, state, fp );
				break;

			case T_INT:
				address = handle_int( tree, address, state, fp );
				break;

			case T_STRING:
			case T_CHAR:
			case T_DOUBLE:
				address = handle_basic_datatype( tree, address, state, fp );
				break;

			case T_NAME:
				address = handle_name( tree, address, state, fp );
				break;

			default:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    ; Skipping type \"%s\"...\n", type_str( tree->type ));
				break;
		}
	}

	return address;
}

tac_t *ptree_to_tlist( parse_node_t *tree ){
	tac_t *ret = NULL;
	parse_node_t *wut = clone_tree( tree );
	gen_state_t state;

	/*
	printf( "-=[ Original:\n" );
	dump_tree( 0, tree );
	*/

	strip_token( wut, T_SEMICOL );
	strip_token( wut, T_COMMA );
	strip_token( wut, T_OPEN_CURL );
	strip_token( wut, T_CLOSE_CURL );
	strip_token( wut, T_OPEN_PAREN );
	strip_token( wut, T_CLOSE_PAREN );

	/*
	printf( "-=[ New: \n" );
	dump_tree( 0, wut );
	*/

	//printf( "-=[ Reduced: \n" );
	wut = trim_tree( wut );
	//dump_tree( 0, wut );

	memset( &state, 0, sizeof( state ));

	FILE *meh = fopen( "testout.s", "w" );
	fprintf( meh, "BITS 64\n" );
	blarg( wut, 0, &state, meh );

	return ret;
}
