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

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "emulator.h"
#include "helper.h"
#include "romloader.h"

void
load_rom(char filename[64], emulator_state** state)
{
  printf("Fake ROM by loading programs into specific locations in memory\n");
  char *line_token, *filecontents, *tofree_contents;

  readfile(&filecontents, filename, false);
  tofree_contents = strdup(filecontents);

  while ((line_token = str_sep(&filecontents, '\n')) != NULL) {
    if (strlen(line_token) > 3) {
      char* addr = str_sep(&line_token, ':');

      uint8_t* code;
      size_t progsize = readfile((char**)&code, line_token, true);
      uint16_t addr_int = strtol(addr, NULL, 16);
      for (int i = addr_int; i < addr_int + progsize; i++) {
        if (i > 0 && i <= 0xffff) {
          (*state)->memory[i] = code[i - addr_int];
        } else {
          printf("romloader: Overflow error: %04x", i);
        }
      }
    }
  }
  free(tofree_contents);
}
