/* codegen.h
 * Defines needed structures and constants for translating parse trees
 * to three address code, and three address code to machine code.
 */

#ifndef _codegen_h
#define _codegen_h

#include <stdio.h>
#include "parse.h"

enum arg_flags {
	ARG_FLAG_DUMP_NONE  = 0,
	ARG_FLAG_DUMP_PARSE = 1,
	ARG_FLAG_DUMP_LEX   = 2,
};

void generate_output_asm( parse_node_t *tree, char *name, enum arg_flags flags );

#endif
