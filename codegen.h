/* codegen.h
 * Defines needed structures and constants for translating parse trees
 * to three address code, and three address code to machine code.
 */

#ifndef _codegen_h
#define _codegen_h

#include <stdio.h>
#include "parse.h"

#define ALLOC_SIZE 8

typedef enum {
	OP_NULL,
	OP_ADD,
	OP_SUB,
	OP_MULT,
	OP_DIV,
} tac_op_t;

typedef struct tac tac_t;
typedef struct tac_var {
	int is_node;

	struct tac *temp; 	// if isn't node
	parse_node_t *node; 	// if is node
} tac_var_t;

struct tac {
	int int_no;

	tac_var_t *first;
	tac_var_t *second;
	tac_op_t op;

	struct tac *next;
	struct tac *prev;
};

/*
// Dynamic array of three address code structs.
typedef struct tac_list {
	parse_node_t *owner;

	tac_t *ops;

	int len;
	int alloced;

	struct tac_list *next;
} tac_list_t;
*/

tac_var_t *new_tac_var( struct tac *val, parse_node_t *node );

tac_t *ptree_to_tlist( parse_node_t *tree );
tac_t *ptot_iter( parse_node_t *node, tac_t *list );

#endif
