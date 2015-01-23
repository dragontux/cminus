#include <stdio.h>
#include <unistd.h>

#include "parse.h"
#include "lex.h"
#include "debug.h"
#include "codegen.h"

void do_help( char **argv ){
	printf( "Usage: %s [-fo file] [-b backend] [-hlp]\n"
		"\t-f:\tSpecify input code file\n"
		"\t-o:\tSpecify file to write output assembly to\n"
		"\t-b:\tSpecify the code generation backend to use\n"
		"\t-h:\tShow this help and exit\n"
		"\t-l:\tDisplay the lexer output for the input code\n"
		"\t-p:\tDisplay the parse tree for the input code\n", argv[0] 
	);
}

int main( int argc, char *argv[] ){
	parse_node_t *meh;
	parse_node_t *move;

	FILE *fp;
	char *filename = NULL;
	char *output_name = "testout.s";
	char *backend = "default";
	char c;

	int	i = 0;

	enum arg_flags flags = ARG_FLAG_DUMP_NONE;

	if ( argc < 2 ){
		do_help( argv );
		return 0;
	}

	while (( c = getopt( argc, argv, "f:b:o:plh" )) != -1 && i++ < argc ){
		switch( c ){
			case 'f':
				filename = argv[++i];
				break;

			case 'b':
				backend = argv[++i];
				break;

			case 'h':
				do_help( argv );
				exit( 0 );
				break;

			case 'l':
				flags |= ARG_FLAG_DUMP_LEX;
				break;

			case 'p':
				flags |= ARG_FLAG_DUMP_PARSE;
				break;

			case 'o':
				output_name = argv[++i];
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

	if ( flags & ARG_FLAG_DUMP_LEX ){
		printf( "-=[ Lexer dump: \n" );
		dump_tree( 0, move );
	}

	move = parse_tokens( meh );

	if ( strcmp( backend, "default" ) == 0 || strcmp( backend, "nasm_x86_64" ) == 0 ){
		generate_output_asm( move, output_name, flags );

	} else {
		die( 1, "Unknown backend \"%s\"\n", backend );
	}

	return 0;
}
