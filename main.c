#include "parse.h"
#include "lex.h"
#include "debug.h"

int main( int argc, char *argv[] ){
	parse_node_t *meh, *move;
	FILE *fp;

	if ( argc < 2 )
		return 1;

	fp = fopen( argv[1], "r" );
	move = meh = lex_file( fp );
	fclose( fp );

	printf( "-=[ Lexer dump: \n" );
	while ( move->next ){
		printf( "got token %d\t", move->type );

		if ( move->type == T_NAME || move->type == T_STRING )
			printf( "\"%s\"", move->data );

		else if ( move->type == T_INT )
			printf( "\"%d\"", *((int *)move->data ));

		printf( "\n" );
		move = move->next;
	}

	move = parse_tokens( meh );
	printf( "-=[ Parse tree dump: \n" );
	dump_tree( 0, move );

	return 0;
}
