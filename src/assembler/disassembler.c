#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "disassembler.h"

uint8_t disassemble_line(char **output_line, uint8_t *memory, uint32_t pc, bool current)
{

  *output_line = malloc(22);
  char arrow_symbol[3] = "";
  if(current) strcpy(arrow_symbol, "->");
  
  if(pc >= 0xfffa) {
    if(pc == 0xfffa)
      sprintf(*output_line, "nmi vector to $%02x%02x", memory[pc + 1], memory[pc]);
    else if(pc == 0xfffc)
      sprintf(*output_line, "reset vector to $%02x%02x", memory[pc + 1], memory[pc]);
    else if(pc == 0xfffe)
      sprintf(*output_line, "irq vector to $%02x%02x", memory[pc + 1], memory[pc]);
    else if(pc > 0xfffe) {
      return 0;
    }
    return 2;
  }
  else {
    int opcode_index;
    int addr_index;
    bool addr_found = false;
    for(opcode_index = 0; opcode_index < 56; opcode_index++) {
      for(addr_index = 0; addr_index < 13; addr_index++) {
        if(memory[pc] == opcode_table[opcode_index][addr_index]) {
          addr_found = true;
          break;
        }
      }
      if(addr_found)
	      break;
    }
    
      
    if(memory[pc] == 0) {
      sprintf(*output_line, "%2s $%04x BRK/NULL", arrow_symbol, pc);
      return 1;
    }
    else {
      int size = op_address_size[addr_index];
      sprintf(*output_line, "%2s $%04x %s", arrow_symbol,
	       pc,
	       opcode_label_table[opcode_index]);
      switch((addressing_mode)addr_index) {
        case absolute:
          sprintf(*output_line,"%s $%02x%02x", *output_line, memory[pc + 2], memory[pc + 1]);
          break;
        case absolute_x:
          sprintf(*output_line,"%s $%02x%02x,X", *output_line, memory[pc + 2], memory[pc + 1]);
          break;
        case absolute_y:
          sprintf(*output_line,"%s $%02x%02x,Y", *output_line, memory[pc + 2], memory[pc + 1]);
          break;
        case indirect:
          sprintf(*output_line,"%s ($%02x%02x)", *output_line, memory[pc + 2], memory[pc + 1]);
          break;
        case indirect_x:
          sprintf(*output_line,"%s ($%02x,X)", *output_line, memory[pc + 1]);
          break;
        case indirect_y:
          sprintf(*output_line,"%s ($%02x),Y", *output_line, memory[pc + 1]);
          break;
        case zeropage_x:
          sprintf(*output_line,"%s $%02x,X", *output_line, memory[pc + 1]);
          break;
        case zeropage_y:
          sprintf(*output_line,"%s $%02x,Y", *output_line, memory[pc + 1]);
          break;
        case zeropage:
        case relative:
          sprintf(*output_line,"%s $%02x", *output_line, memory[pc + 1]);
          break;
        case immediate:
          sprintf(*output_line,"%s #$%02x", *output_line, memory[pc + 1]);
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
