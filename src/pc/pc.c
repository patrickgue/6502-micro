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
  /*for(int i = 0; i < tokens_count; i++) {
    switch(tokens[i].type) {
    case PREPR:
      printf("PREPR(%s)\n", tokens[i].text);
      break;
    case ARRITH:
      printf("ARRITH(%s)\n", tokens[i].text);
      break;
    case NUMBER:
      printf("NUMBER(%s)\n", tokens[i].text);
      break;
    case NUMBER_HEX:
      printf("NUMBER_HEX(%s)\n", tokens[i].text);
      break;
    case STRING:
      printf("STRING(%s)\n", tokens[i].text);
      break;
    case LABEL:
      printf("LABEL(%s)\n", tokens[i].text);
      break;
    case FUNC:
      printf("FUNC(%s)\n", tokens[i].text);
      break;
    case IF:
      printf("IF\n");
      break;
    case ELSE:
      printf("ELSE\n");
      break;
    case WHILE:
      printf("WHILE(%s)\n", tokens[i].text);
      break;
    case SCOPE_OPEN:
      printf("SCOPE_OPEN\n");
      break;
    case SCOPE_CLOSE: 
      printf("SCOPE_CLOSE\n");
      break;
    case ARG_OPEN:
      printf("ARG_OPEN\n");
      break;
    case ARG_CLOSE:
      printf("ARG_CLOSE\n");
      break;
    case LET:
      printf("LET(%s)\n",tokens[i].text);
      break;
    case STMT_SEP:
      printf("STMT_SEP\n");
      break;
    case VAR_SEP:
      printf("VAR_SEP\n");
      break;
    case ASSING:
      printf("ASSIGN\n");
      break;
    default:
      break;
    }
  }*/

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