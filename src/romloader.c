/*
  romloader.c  subroutine for injecting programs into RAM of emulator
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
#include <stdlib.h>

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include <Z/hardware/CPU/architecture/6502.h>

#include "romloader.h"
#include "helper.h"

void load_rom(char filename[64], zuint8 * memory)
{
  printf("Fake ROM by loading programs into specific locations in memory\n");
  char *line_token, *filecontents, *word_token, *tofree_contents, *tofree_line;

  readfile(&filecontents, filename, false);
  printf("Success reading file ");//(%s)", filecontents);
  tofree_contents = strdup(filecontents);

  line_token = strsep(&filecontents, "\n");
  zuint16 reset_vector = strtol(line_token, NULL, 16);
  memory[0xfffc] = reset_vector & 0x00ff;
  memory[0xfffd] = (reset_vector & 0xff00) >> 8;
  
  while((line_token = strsep(&filecontents, "\n")) != NULL) {
    tofree_line = strdup(line_token);
    if(strlen(line_token) > 3) {
      printf("%s\n", line_token);

      char *addr = strsep(&line_token, ":");
      char *filename = strsep(&line_token, ":");
      free(tofree_line);
    
      zuint8 *code;
      zusize progsize = readfile((char**)&code, filename, true);
      zuint16 addr_int = strtol(addr, NULL, 16);
      for(int i = addr_int; i < addr_int + progsize; i++) {
	printf("%04x %02x\n", i, code[i-addr_int]);
	if(i > 0 && i <= 0xffff) {
	  memory[i] = code[i-addr_int];
	}
	else {
	  printf("romloader: Overflow error: %04x", i);
	}
      }
 
    }
  }
  free(tofree_contents);
  printf("done loading fake rom\n");
}
