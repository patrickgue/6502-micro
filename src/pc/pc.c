/*
  pc.c  Compiler for PL (Primitive 6502 optimized Language) 
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


#include <stdio.h>
#include <regex.h>
#include <stdlib.h>

#include "../helper.h"
#include "token.h"
#include "tree.h"
#include "pc.h"


int 
main(int argc, char **argv) 
{
  if(argc < 2) {
    usage();
    exit(1);
  }
  char *buffer;
  readfile(&buffer, argv[1], false);
  token *tokens;
  int tokens_count = tokenize(buffer, &tokens);
  

  tree_scope *root_scope = (tree_scope*) malloc(sizeof(tree_scope));
  root_scope->statements = malloc(0);
  root_scope->statements_length = 0;
  build_tree(&root_scope, tokens, tokens_count);
  
}


void
usage()
{
  printf("Usage:\npc [input file] [[output file]]\n");
}