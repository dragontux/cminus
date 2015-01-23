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
				( tree->down != NULL && !tree->down->next )
				&& tree->type != T_CALL ){

			ret = trim_tree( tree->down );
			free( tree );

		} else if ( tree->down != NULL && !tree->down->next
				&& tree->type != T_STATEMNT && tree->type != T_CALL ){

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

unsigned gen_code( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp );

unsigned gen_decl_list( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	address = gen_code( tree->down, address + 1, state, fp );
	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_var_decl( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
	name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));
	name_decl_t *move;

	move = find_name_no_recurse( tree->down->next->data, state );

	if ( move ){
		fprintf( stderr, "Error: symbol \"%s\" already defined in this scope\n", tree->down->next->data );
	}

	state->num_vars++;

	new_name->type = tree->down->data;
	new_name->name = tree->down->next->data;
	new_name->number = state->num_vars;
	new_name->state = state;

	if ( state->flags & STATE_IN_FUNCTION ){
		fprintf( fp, "    sub rsp, 8 ; %s %s\n",
				tree->down->data, tree->down->next->data );

		new_name->placement = VAR_PLACE_LOCAL;

	} else {
		fprintf( fp, "section .bss\n" );
		fprintf( fp, "global %s\n", tree->down->next->data );
		fprintf( fp, "%s: resq 1 ; %s\n",
				tree->down->next->data, tree->down->data );

		new_name->placement = VAR_PLACE_GLOBAL;
	}

	add_name( state, new_name );

	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_func_decl( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
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

	if ( strcmp( tree->down->data, "extern" ) == 0 ){
		fprintf( fp, "extern %s\n", tree->down->next->data );

	} else {
		fprintf( fp, "section .text\n" );
		fprintf( fp, "global %s\n", tree->down->next->data );
		fprintf( fp, "%s: ; returns %s\n", tree->down->next->data, tree->down->data );

		fprintf( fp, "    push rbp\n" );
		fprintf( fp, "    mov rbp, rsp\n" );

		address = gen_code( tree->down->next->next, address + 1, &newscope, fp ) + 1;

		fprintf( fp, ".function_end:\n" );
		fprintf( fp, "    mov rsp, rbp\n" );
		fprintf( fp, "    pop rbp\n" );
		fprintf( fp, "    ret\n" );
	}

	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_param_decl_list( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
	address = gen_code( tree->down, address + 1, state, fp );
	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_param_decl( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ) {
	name_decl_t *new_name = calloc( 1, sizeof( name_decl_t ));

	new_name->type = tree->down->data;
	new_name->name = tree->down->next->data;
	new_name->placement = VAR_PLACE_PARAMETER;
	new_name->number = state->num_params;
	new_name->state = state;
	add_name( state, new_name );

	state->num_params++;

	//	fprintf( fp, "    ;%4d > \n", address );
	//fprintf( fp, "    ; parameter \"%s\" of type \"%s\" at [rbp-%u]\n",
	//tree->down->next->data, tree->down->data, state->num_params * 8 );

	address = gen_code( tree->next, address + 1, state, fp );// + 1;

	return address;
}

unsigned gen_statement( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	address = gen_code( tree->down, address + 1, state, fp );
	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_iter_statement( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned test_start, test_addr;

	test_start = address;

	fprintf( fp, "    ; iterator statement, start at %u\n", test_start );
	fprintf( fp, ".while_%u:\n", test_start );

	test_addr = gen_code( tree->down->next, address + 1, state, fp );
	address = test_addr + 1;

	fprintf( fp, "    test rax, rax ; check result from %u\n", test_addr );
	fprintf( fp, "    jz .end_while_%u\n", test_start );

	address = gen_code( tree->down->next->next, address + 1, state, fp );
	address++;

	fprintf( fp, "    jmp .while_%u\n", test_start );
	fprintf( fp, "    ; end of iterator at %u\n", test_start );
	fprintf( fp, ".end_while_%u:\n", test_start );

	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_assignment_expression(
		parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp )
{
	unsigned op;

	// TODO: Make sure that tree->down->data is actually a name symbol,
	//       just in case
	name_decl_t *name = find_name( tree->down->data, state );

	if ( name ){
		op = gen_code( tree->down->next->next, address + 1, state, fp );
		address = op + 1;

		switch( name->placement ){
			case VAR_PLACE_GLOBAL:
				fprintf( fp, "    mov [%s], rax\n", tree->down->data );
				break;

			case VAR_PLACE_PARAMETER:
				fprintf( fp, "    mov [rbp+%u], rax\n", (name->number + 1) * 8 );
				break;

			case VAR_PLACE_LOCAL:
				fprintf( fp, "    mov [rbp-%u], rax\n", name->number * 8 );
				break;

			default:
				break;
		}

		//fprintf( fp, "    ;%4d > \n", address );
		//fprintf( fp, "    ; End of expression, op are %u\n", op );
	}

	return address;
}

unsigned gen_expression( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned op1, op2;

	//fprintf( fp, "    ;%4d > \n", address );
	//fprintf( fp, "    ; Have expression, operation is %s\n", type_str( tree->down->next->type ));

	if ( tree->down->next->type == T_EQUALS ){
		address = gen_assignment_expression( tree, address + 1, state, fp );

	} else {
		op1 = gen_code( tree->down, address + 1, state, fp );
		address = op1 + 1;

		fprintf( fp, "    mov rbx, rax\n" );
		fprintf( fp, "    push rbx\n" );

		op2 = gen_code( tree->down->next->next, address + 1, state, fp );
		address = op2 + 1;

		fprintf( fp, "    pop rbx\n" );

		switch( tree->down->next->type ){
			case T_PLUS:
				fprintf( fp, "    add rax, rbx\n" );
				address++;
				break;

			case T_MINUS:
				fprintf( fp, "    sub rbx, rax\n" );
				fprintf( fp, "    mov rax, rbx\n" );
				address++;
				break;

			case T_STAR:
				fprintf( fp, "    mul rbx\n" );
				address++;
				break;

			case T_SLASH:
				fprintf( fp, "    mov rcx, rax\n" );
				fprintf( fp, "    mov rax, rbx\n" );
				fprintf( fp, "    mov rdx, 0\n" );
				fprintf( fp, "    div rcx\n" );
				address++;
				break;

			// TODO: find more efficient way to do comparisons
			case T_LESS_THAN:
				fprintf( fp, "    cmp rbx, rax\n" );
				fprintf( fp, "    jge .cmp_false_%u\n", address );
				fprintf( fp, "    mov rax, 1\n" );
				fprintf( fp, "    jmp .cmp_end_%u\n", address );
				fprintf( fp, ".cmp_false_%u:\n", address );
				fprintf( fp, "    mov rax, 0\n" );
				fprintf( fp, ".cmp_end_%u:\n", address );
				address++;
				break;

			case T_GREATER_THAN:
				fprintf( fp, "    cmp rbx, rax\n" );
				fprintf( fp, "    jle .cmp_false_%u\n", address );
				fprintf( fp, "    mov rax, 1\n" );
				fprintf( fp, "    jmp .cmp_end_%u\n", address );
				fprintf( fp, ".cmp_false_%u:\n", address );
				fprintf( fp, "    mov rax, 0\n" );
				fprintf( fp, ".cmp_end_%u:\n", address );
				address++;
				break;

			default:
				break;
		}

		//fprintf( fp, "    ;%4d > \n", address );
		//fprintf( fp, "    ; End of expression, ops are %u and %u\n", op1, op2 );
	}

	return address;
}

unsigned gen_select( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned test_addr, start, body_addr, else_addr = 0, end;

	test_addr = gen_code( tree->down->next, address + 1, state, fp );
	address = test_addr + 1;

	start = address;

	fprintf( fp, ".if_%u:\n", start );
	fprintf( fp, "    test rax, rax ; check result from %u\n", test_addr );
	fprintf( fp, "    jz .else_if_%u\n", start );

	body_addr = gen_code( tree->down->next->next, address + 1, state, fp );
	address = body_addr + 1;

	fprintf( fp, "    jmp .end_if_%u\n", start );
	fprintf( fp, ".else_if_%u:\n", start );

	if ( tree->down->next->next->next ){
		else_addr = gen_code( tree->down->next->next->next->next, address + 1, state, fp );
		address = else_addr + 1;
	}

	fprintf( fp, ".end_if_%u:\n", start );

	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_return( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	unsigned return_addr;

	return_addr = gen_code( tree->down->next, address + 1, state, fp );
	address = return_addr + 1;

	//	fprintf( fp, "    ;%4d > \n", address );
	//fprintf( fp, "    ; Have return statement, return value at %u\n", return_addr );

	fprintf( fp, "    jmp .function_end\n" );

	address = gen_code( tree->next, address + 1, state, fp );

	return address;
}

unsigned gen_variable( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	fprintf( stderr, "Warning: array indexes not implemented yet\n" );

	fprintf( fp, "    ;%4d > \n", address );
	fprintf( fp, "    ; Have variable \"%s\", index %d\n",
			tree->down->data, *(int *)tree->down->next->next->data );

	return address;
}

unsigned gen_call( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	parse_node_t *move;
	unsigned param;
	unsigned i;

	char *saveregs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9", NULL };

	for ( move = tree->down->next, i = 0; move; move = move->next, i++ ){
		if ( move->type == T_ARGS_LIST )
			move = move->down;

		param = gen_code( move, address + 1, state, fp );
		address = param + 1;

		fprintf( fp, "    push %s\n", saveregs[i] );
		fprintf( fp, "    mov %s, rax ; store %d as parameter %u\n", saveregs[i], param, i );

		address++;
	}

	fprintf( fp, "    call %s\n", tree->down->data );

	for ( ; i; i-- ){
		fprintf( fp, "    pop %s\n", saveregs[i - 1] );
	}

	return address;
}

unsigned gen_name( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	name_decl_t *name = find_name( tree->data, state );

	if ( name ){
		switch( name->placement ){
			case VAR_PLACE_GLOBAL:
				fprintf( fp, "    mov rax, [%s]\n", tree->data );
				break;

			case VAR_PLACE_PARAMETER:
				//fprintf( fp, "    mov rax, [rbp+%u]\n", (name->number + 1) * 8 );
				fprintf( fp, "    mov rax, %s\n",
						(char *[]){ "rdi", "rsi", "rdx", "rcx", "r8", "r9" }[ name->number ]);
				break;

			case VAR_PLACE_LOCAL:
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

unsigned gen_int( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	fprintf( fp, "    mov rax, %u\n", *(int *)tree->data );

	return address;
}

unsigned gen_basic_datatype( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	/* TODO: handle stuff here
	printf( "    ;%4d > \n", address );
	printf( "    ; Have basic datatype \"%s\"\n", type_str( tree->type ));
	*/

	return address;
}

unsigned gen_code( parse_node_t *tree, unsigned address, gen_state_t *state, FILE *fp ){
	if ( tree ){
		switch( tree->type ){
			case T_DECL_LIST:
				address = gen_decl_list( tree, address, state, fp );
				break;

			case T_VAR_DECL:
				address = gen_var_decl( tree, address, state, fp );
				break;

			case T_FUN_DECL:
				address = gen_func_decl( tree, address, state, fp );
				break;

			case T_PARAM_DECL_LIST:
				address = gen_param_decl_list( tree, address, state, fp );
				break;

			case T_PARAM_DECL:
				address = gen_param_decl( tree, address, state, fp );
				break;

			case T_STATEMNT_LIST:
			case T_COMP_STATEMNT:
			case T_STATEMNT:
				address = gen_statement( tree, address, state, fp );
				break;

			case T_ITER_STATEMNT:
				address = gen_iter_statement( tree, address, state, fp );
				break;

			case T_EXPR:
			case T_ADD_EXPR:
			case T_TERM:
			case T_SIMPLE_EXPR:
				address = gen_expression( tree, address, state, fp );
				break;

			case T_SELECT_STATEMNT:
				address = gen_select( tree, address, state, fp );
				break;

			case T_RETURN_STATEMNT:
				address = gen_return( tree, address, state, fp );
				break;

			case T_VAR:
				address = gen_variable( tree, address, state, fp );
				break; 

			case T_CALL:
				address = gen_call( tree, address, state, fp );
				break;

			case T_INT:
				address = gen_int( tree, address, state, fp );
				break;

			case T_STRING:
			case T_CHAR:
			case T_DOUBLE:
				address = gen_basic_datatype( tree, address, state, fp );
				break;

			case T_NAME:
				address = gen_name( tree, address, state, fp );
				break;

			default:
//				fprintf( fp, "    ;%4d > \n", address );
				fprintf( fp, "    ; Skipping type \"%s\"...\n", type_str( tree->type ));
				break;
		}
	}

	return address;
}

void generate_output_asm( parse_node_t *tree, char *name, enum arg_flags flags ){
	parse_node_t *wut = clone_tree( tree );
	gen_state_t state;

	if ( flags & ARG_FLAG_DUMP_PARSE ){
		printf( "-=[ Original:\n" );
		dump_tree( 0, tree );
	}

	strip_token( wut, T_SEMICOL );
	strip_token( wut, T_COMMA );
	strip_token( wut, T_OPEN_CURL );
	strip_token( wut, T_CLOSE_CURL );
	strip_token( wut, T_OPEN_PAREN );
	strip_token( wut, T_CLOSE_PAREN );

	if ( flags & ARG_FLAG_DUMP_PARSE ){
		printf( "-=[ Tokens stripped: \n" );
		dump_tree( 0, wut );
	}

	wut = trim_tree( wut );

	if ( flags & ARG_FLAG_DUMP_PARSE ){
		printf( "-=[ Reduced: \n" );
		dump_tree( 0, wut );
	}

	memset( &state, 0, sizeof( state ));

	FILE *meh = fopen( name, "w" );
	fprintf( meh, "BITS 64\n" );
	gen_code( wut, 0, &state, meh );
}
