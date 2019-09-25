/*
  disassembler_cli.c  Commandline interface for disassembler.c 
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
#include <stdbool.h>

#include "disassembler.h"
#include "../helper.h"

void usage() {
  printf("Usage:\ndas65 [FILE]\n");
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        usage();
        return -1;
    }

    uint8_t *buffer;
    char *line;

    int size = readfile((char**)&buffer, argv[1], true), i;

    while(i < size) {
        i+= disassemble_line(&line, buffer, i, false);
        printf("%s\n", line);
    }

    return 0;
}