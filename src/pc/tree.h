/*
  tree.h  Headerfile for tree.c 
  Copyright (C) 2019 Patrick Günthard

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TREE_H
#define TREE_H

#include <stdbool.h>
#include "token.h"



struct s_tree_statement;
typedef struct s_tree_statement tree_statement;

struct s_tree_scope;
typedef struct s_tree_scope tree_scope;

struct s_tree_scope {
    tree_statement *statements;
    unsigned int statements_length;
};

enum e_tree_statement_type {T_DEF, T_ASSIGN, T_CONTROL, T_ARRITH, T_ATOMIC, T_ARGS, T_NONE};
typedef enum e_tree_statement_type tree_statement_type;

enum e_t_control_type {TC_WHILE, TC_IF, TC_FUNC};
typedef enum e_t_control_type t_control_type;


enum e_tree_atomic_type {A_NUMBER,A_STRING,A_REFERENCE};
typedef enum e_tree_atomic_type tree_atomic_type;

struct s_tree_atomic {
    tree_atomic_type type;
    char reference[32];
    int number;
    char string[128];
};

typedef struct s_tree_atomic tree_atomic;

struct s_tree_statement {
    tree_statement_type statement_type;
    t_control_type control_type;
    tree_atomic *atomic;
    void *left;
    tree_statement_type left_type;
    void *right;
    tree_statement_type right_type;
    tree_scope *sub_scope;
    tree_scope *args_scope;
};


void
build_tree(tree_scope **, token *, int);

void 
build_scope(tree_scope **, token_scope_tree *, int, int);

void 
add_statement(tree_scope **, tree_statement*);

tree_statement *
build_statement(tree_scope **, token_scope_tree *, int, int);

tree_statement * 
create_empty_statement(tree_statement_type);

tree_statement *
build_atomic_statement(token_scope_tree *, int);


int 
contains_token_type(token_type, token_scope_tree *, int, int);


#endif
