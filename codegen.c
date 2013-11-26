#include "parse.h"

extern char *type_str( token_type_t );

int x86_codegen( parse_node_t *tokens ){
	if ( tokens ){
		printf( "%s\n", type_str( tokens->type ));
		x86_codegen( tokens->down );
		x86_codegen( tokens->next );
	}
	
	return 0;
}
