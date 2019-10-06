/*
  disassembler.c  6502 Disassembler
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "disassembler.h"

uint8_t
disassemble_line(char** output_line, uint8_t* memory, uint32_t pc, bool current)
{

  *output_line = malloc(22);
  char arrow_symbol[3] = "";
  if (current)
    strcpy(arrow_symbol, "->");

  if (pc >= 0xfffa) {
    if (pc == 0xfffa)
      sprintf(
        *output_line, "nmi vector to $%02x%02x", memory[pc + 1], memory[pc]);
    else if (pc == 0xfffc)
      sprintf(
        *output_line, "reset vector to $%02x%02x", memory[pc + 1], memory[pc]);
    else if (pc == 0xfffe)
      sprintf(
        *output_line, "irq vector to $%02x%02x", memory[pc + 1], memory[pc]);
    else if (pc > 0xfffe) {
      return 0;
    }
    return 2;
  } else {
    int opcode_index;
    int addr_index;
    bool addr_found = false;
    for (opcode_index = 0; opcode_index < 56; opcode_index++) {
      for (addr_index = 0; addr_index < 13; addr_index++) {
        if (memory[pc] == opcode_table[opcode_index][addr_index]) {
          addr_found = true;
          break;
        }
      }
      if (addr_found)
        break;
    }

    if (memory[pc] == 0) {
      sprintf(*output_line, "%2s $%04x BRK/NULL", arrow_symbol, pc);
      return 1;
    } else {
      int size = op_address_size[addr_index];
      sprintf(*output_line,
              "%2s $%04x %s",
              arrow_symbol,
              pc,
              opcode_label_table[opcode_index]);
      switch ((addressing_mode)addr_index) {
        case absolute:
          sprintf(*output_line,
                  "%s $%02x%02x",
                  *output_line,
                  memory[pc + 2],
                  memory[pc + 1]);
          break;
        case absolute_x:
          sprintf(*output_line,
                  "%s $%02x%02x,X",
                  *output_line,
                  memory[pc + 2],
                  memory[pc + 1]);
          break;
        case absolute_y:
          sprintf(*output_line,
                  "%s $%02x%02x,Y",
                  *output_line,
                  memory[pc + 2],
                  memory[pc + 1]);
          break;
        case indirect:
          sprintf(*output_line,
                  "%s ($%02x%02x)",
                  *output_line,
                  memory[pc + 2],
                  memory[pc + 1]);
          break;
        case indirect_x:
          sprintf(*output_line, "%s ($%02x,X)", *output_line, memory[pc + 1]);
          break;
        case indirect_y:
          sprintf(*output_line, "%s ($%02x),Y", *output_line, memory[pc + 1]);
          break;
        case zeropage_x:
          sprintf(*output_line, "%s $%02x,X", *output_line, memory[pc + 1]);
          break;
        case zeropage_y:
          sprintf(*output_line, "%s $%02x,Y", *output_line, memory[pc + 1]);
          break;
        case zeropage:
        case relative:
          sprintf(*output_line, "%s $%02x", *output_line, memory[pc + 1]);
          break;
        case immediate:
          sprintf(*output_line, "%s #$%02x", *output_line, memory[pc + 1]);
          break;
        case accumlator:
        case implied:
          // nothing to do
          break;
      }
      return size;
    }
  }
}
