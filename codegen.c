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

		} else if ( tree->down != NULL && !tree->down->next ){
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

tac_t *ptree_to_tlist( parse_node_t *tree ){
	tac_t *ret = NULL;
	parse_node_t *wut = clone_tree( tree );

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

	//ret = ptot_iter( wut, NULL );
	//for ( ; ret->prev; ret = ret->prev );
	//dump_tac_list( ret );

	return ret;
}

int x86_codegen( tac_t *list ){
	
	return 0;
}
