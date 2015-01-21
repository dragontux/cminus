/* codegen.h
 * Defines needed structures and constants for translating parse trees
 * to three address code, and three address code to machine code.
 */

#ifndef _codegen_h
#define _codegen_h

#include <stdio.h>
#include "parse.h"

void generate_output_asm( parse_node_t *tree, char *name );

#endif
