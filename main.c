#include <stdio.h>
#include <unistd.h>

#include "parse.h"
#include "lex.h"
#include "debug.h"

void do_help( char **argv ){
	printf( "Usage: %s [-f file] [-h]\n"
		"	-f:	Specify input code file\n"
		"	-h:	Show this help and exit\n", argv[0] 
	);
}

int main( int argc, char *argv[] ){
	parse_node_t *meh, *move;
	char	*filename = NULL,
		c;
	FILE	*fp;
	int	i = 0,
		lex_dump = 0,
		parse_dump = 0;

	if ( argc < 2 ){
		do_help( argv );
		return 0;
	}

	while (( c = getopt( argc, argv, "f:plh" )) != -1 && i++ < argc ){
		switch( c ){
			case 'f':
				filename = argv[++i];
				break;
			case 'h':
				do_help( argv );
				exit( 0 );
				break;
			case 'l':
				lex_dump = 1;
				break;
			case 'p':
				parse_dump = 1;
				break;
		}
	}

	if ( !filename ){
		do_help( argv );
		return 0;
	}

	fp = fopen( filename, "r" );
	move = meh = lex_file( fp );
	fclose( fp );

	if ( lex_dump ){
		printf( "-=[ Lexer dump: \n" );
		dump_tree( 0, move );
	}

	move = parse_tokens( meh );
	if ( parse_dump ){
		printf( "-=[ Parse tree dump: \n" );
		dump_tree( 0, move );
	}

	return 0;
}
