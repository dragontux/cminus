#include <cminus/lex.h>

int cline = 1;
parse_node_t *get_token( FILE *fp ){
	parse_node_t *ret = NULL;
	bool	has_token = false;
		/* TODO: Handle escapes
		,
		quote	= false;
		escape	= false;
		*/
	char 	c, 
		*temp;
	int 	i,
		j,
		*blarg;

	ret = calloc( 1, sizeof( parse_node_t ));
	ret->type = T_NULL;

	while ( has_token == false ){
		c = getc( fp );
		//printf( "[get_token] got char '%c'\n", c );

		if ( feof( fp )){
			break;

		} else if ( c == ' ' || c == '\t' ){
			continue;

		} else if ( c == '\n' ){
			cline++;
			continue;

		} else if (	( 'A' <= c && c <= 'Z' )||
				( 'a' <= c && c <= 'z' )||
				c == '_' )
			{

			has_token = true;

			temp = malloc( 256 );
			for ( i = 0; (	( 'A' <= c && c <= 'Z' 	) || 
					( 'a' <= c && c <= 'z' 	) || 
					( '0' <= c && c <= '9' 	) ||
					( c == '_' 		))&& 
					!feof( fp ) && 
					i < 256;
					i++, c = getc( fp ))
			{
				if ( c == '\n' )
					cline++;

				temp[i] = c;
			}

			ungetc( c, fp );
			temp[i] = 0;

			if ( strcmp( temp, "if" ) == 0 )
				ret->type = T_IF;
			else if ( strcmp( temp, "else" ) == 0 )
				ret->type = T_ELSE;
			else if ( strcmp( temp, "while" ) == 0 )
				ret->type = T_WHILE;
			else if ( strcmp( temp, "for" ) == 0 )
				ret->type = T_FOR;
			else if ( strcmp( temp, "switch" ) == 0 )
				ret->type = T_SWITCH;
			else if ( strcmp( temp, "return" ) == 0 )
				ret->type = T_RETURN;
			else 
				ret->type = T_NAME;

			ret->size = i;
			ret->data = temp;

		} else if ( '0' <= c && c <= '9' ){
			has_token = true;

			for ( j = 0; ( '0' <= c && c <= '9' ) && !feof( fp );
					c = getc( fp ))
			{
				j = j * 10 + c - '0';
			}

			ungetc( c, fp );

			blarg = malloc( sizeof( int ));
			*blarg = j;

			ret->type = T_INT;
			ret->size = sizeof( int );
			ret->data = blarg;

		} else if ( c == '"' ){
			has_token = true;

			temp = malloc( 256 );
			c = getc( fp );
			for ( i = 0; c != '"' && !feof( fp ) && i < 256;
					i++, c = getc( fp ))
			{
				temp[i] = c;
			}

			temp[i] = 0;

			ret->type = T_STRING;
			ret->size = i;
			ret->data = temp;

		/* TODO: Implement the following conditions as a table lookup, this looks dumb */
		} else if ( c == '(' ){
			has_token = true;
			ret->type = T_OPEN_PAREN;

		} else if ( c == ')' ){
			has_token = true;
			ret->type = T_CLOSE_PAREN;

		} else if ( c == '[' ){
			has_token = true;
			ret->type = T_OPEN_BRACK;

		} else if ( c == ']' ){
			has_token = true;
			ret->type = T_CLOSE_BRACK;

		} else if ( c == '{' ){
			has_token = true;
			ret->type = T_OPEN_CURL;

		} else if ( c == '}' ){
			has_token = true;
			ret->type = T_CLOSE_CURL;

		} else if ( c == '<' ){
			has_token = true;
			ret->type = T_LESS_THAN;

		} else if ( c == '>' ){
			has_token = true;
			ret->type = T_GREATER_THAN;

		} else if ( c == ';' ){
			has_token = true;
			ret->type = T_SEMICOL;
		
		} else if ( c == '=' ){
			has_token = true;
			ret->type = T_EQUALS;

		} else if ( c == '+' ){
			has_token = true;
			ret->type = T_PLUS;

		} else if ( c == '-' ){
			has_token = true;
			ret->type = T_MINUS;

		} else if ( c == '*' ){
			has_token = true;
			ret->type = T_STAR;

		} else if ( c == '/' ){
			has_token = true;
			ret->type = T_SLASH;

			/* TODO Add checking for comments */
		} else if ( c == ',' ){
			has_token = true;
			ret->type = T_COMMA;

		} else if ( c == '!' ){
			has_token = true;
			ret->type = T_EXCLAIM;

		} else { 
			die( 1, "Line %d: Lexer error, unknown token \"%c\".\n", cline, c );

		}
	}

	return ret;
}

parse_node_t *lex_file( FILE *fp ){
	parse_node_t	*ret,
			*last,
			*move;

	ret = last = move = get_token( fp );

	while ( move->type != T_NULL ){
		last = move;
		move = get_token( fp );

		move->prev = last;
		last->next = move;
	}

	return ret;
}

