#ifndef _lex_h
#define _lex_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "parse.h"

parse_node_t *get_token( FILE *fp );
parse_node_t *lex_file( FILE *fp );

#endif
