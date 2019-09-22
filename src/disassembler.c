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
	 
      if(size == 3) {
	sprintf(*output_line,"%s $%02x%02x", *output_line, memory[pc + 2],
		 memory[pc + 1]);
      }
      else if(size == 2) {
	sprintf(*output_line,"%s $%02x", *output_line, memory[pc + 1]);
      }
      return size;
    }
  }
}
