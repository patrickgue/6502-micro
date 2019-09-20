/*
  nemu_debug.c  debugger functions for nemu.c 
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
#include <ncurses.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "assembler.h"
#include "emulator.h"

#include "nemu_debug.h"


void update_rw_buffer(char new_line[11]) {
  for(int i = 0; i < rw_buffer_length -1; i++) {
    strcpy(rw_buffer[i], rw_buffer[i+1]);
  }
  strcpy(rw_buffer[rw_buffer_length -1], new_line);
}

void display_rw_buffer(int y, int x)
{
  for(int i = 0; i < rw_buffer_length; i++) {
    mvprintw(y+i,x,rw_buffer[i]);
  }
}


/* display memory page (54/17) */
void display_memory(int y_offset,int x_offset, emulator_state *state, uint8_t page) {
  mvprintw(y_offset,x_offset,"Memory (Page %02x)", page);
  for(int x = 0; x < 0x10; x++) {
    mvprintw(y_offset+1,x_offset + 7 + (x * 3), "%02x",x);
    mvprintw(y_offset+2,x_offset + 7 + (x * 3), "---",x);
  }
  for(int y = 0; y < 0x10; y++) {
    mvprintw(y_offset + 3 + y, x_offset, "%04x | ", (page * 0x0100) + (y*0x10));
    for(uint8_t x = 0; x < 0x10; x++) {
      mvprintw(y_offset+3+y,x_offset + 7 + (x * 3), "%02x",state->memory[(page * 0x100) + x+(y*0x10)]);
    }
  }
  if(state->cpu->state.pc >= (page * 0x100) && state->cpu->state.pc < (page * 0x100) + 0x100) {
    int lb = state->cpu->state.pc & 0x00ff;
    int x = lb & 0x0f;
    int y = (lb & 0xf0) / 0x10;
    mvprintw(y_offset + 3 + 0x10 + 1, x_offset, "%02x  %02x (%02x)", x,y, lb);
    mvprintw(y_offset+3+y, x_offset + 7 + (x * 3) - 1, "*");
  }
}

/* display disassembled code (5/12) */
void display_disassemble(int y, int x, emulator_state* state)
{
  int pc = state->cpu->state.pc;
  int pc_offset;
  for(int i = 0; i < 20; i++) {
    if(pc + pc_offset == 0xfffa)
      mvprintw(y+i,x,"nmi vector to $%02x%02x", state->memory[pc + pc_offset], state->memory[pc + pc_offset + 1]);
    else if(state->cpu->state.pc + pc_offset == 0xfffc)
      mvprintw(y+i,x,"reset vector to $%02x%02x", state->memory[pc + pc_offset], state->memory[pc + pc_offset + 1]);
    else if(state->cpu->state.pc + pc_offset == 0xfffe)
      mvprintw(y+i,x,"irq vector to $%02x%02x", state->memory[pc + pc_offset], state->memory[pc + pc_offset + 1]);
    else if(pc + pc_offset > 0xfffe) {
      break;
    }
    else {
      int opcode_index;
      int addr_index;
      bool addr_found = false;
      for(opcode_index = 0; opcode_index < 56; opcode_index++) {
	for(addr_index = 0; addr_index < 13; addr_index++) {
	  if(state->memory[pc + pc_offset] == opcode_table[opcode_index][addr_index]) {
	    addr_found = true;
	    break;
	  }
	}
	if(addr_found)
	  break;
      }
      
      
      if(state->memory[pc + pc_offset] == 0) {
	mvprintw(y+i,x,"%2s $%04x BRK/NULL", (i == 0 ? "->" : ""), state->cpu->state.pc + pc_offset);
	pc_offset++;
      }
      else {
	int size = op_address_size[addr_index];
	mvprintw(y+i,x,"%2s $%04x %s", (i == 0 ? "->" : ""),
		 pc + pc_offset,
		 opcode_label_table[opcode_index]);
	 
	if(size == 3) {
	  mvprintw(y+i,x+13,"$%02x%02x", state->memory[pc + pc_offset + 2],
		   state->memory[pc + pc_offset + 1]);
	}
	else if(size == 2) {
	  mvprintw(y+i,x+13,"$%02x", state->memory[pc + pc_offset + 1]);
	}
	pc_offset += size;
      }
    }
  }
}

/* display state of cpu (6/33) */
void display_state(int y, int x, emulator_state* state)
{
  mvprintw(y,x,"Registers");
  mvprintw(y+1,x,"A: %02x  X: %02x  Y: %02x  PC: %04x",
	   state->cpu->state.a,
	   state->cpu->state.x,
	   state->cpu->state.y,
	   state->cpu->state.pc);
  mvprintw(y+3,x,"Flags");
  mvprintw(y+4,x,"| N | V |   | B | D | I | Z | C |");
  mvprintw(y+5,x,"  %d   %d       %d   %d   %d   %d   %d",
	   (state->cpu->state.p & 0b10000000) >> 7,
	   (state->cpu->state.p & 0b01000000) >> 6,
	   (state->cpu->state.p & 0b00010000) >> 4,
	   (state->cpu->state.p & 0b00001000) >> 3,
	   (state->cpu->state.p & 0b00000100) >> 2,
	   (state->cpu->state.p & 0b00000010) >> 1,
	   (state->cpu->state.p & 0b00000001));
  }

void debug_bus_read(uint16_t addr, uint8_t data) {
  char new_line[RW_BUFFER_LINE_LEN];
  sprintf(new_line, "r %04x %02x", addr, data);
  update_rw_buffer(new_line);
}

void debug_bus_write(uint16_t addr, uint8_t data) {
  char new_line[RW_BUFFER_LINE_LEN];
  sprintf(new_line, "w %04x %02x", addr, data);
  update_rw_buffer(new_line);
}

