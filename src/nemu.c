#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>
#include <string.h>

#include "emulator.h"
#include "license.h"
#include "assembler.h"

#include "nemu.h"

#define CLOCKSPEED 2000000//2000000

#define RW_BUFFER_LINE_LEN 11 // rw 1 + space 1 + addr 4 + space 1 + data 2 + zero 1
char rw_buffer[20][RW_BUFFER_LINE_LEN] = {"","","","","","","","","",""};
int  rw_buffer_length = 20;


int main(int argc, char **argv)
{
  printf("%s", rw_buffer[0]);
  
  /* init ncurses */
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  int CWIDTH = COLS > 120 ? 120 : COLS;
  int CHEIGHT = LINES > 40 ? 40 : LINES;
  

  /* show init menu */
  int step_count = init_menu();

  /* init_emulator */
  emulator_state *state;
  init_emulator(&state, CLOCKSPEED);
  state->debug_read = &debug_bus_read;
  state->debug_write = & debug_bus_write;
  m6502_power(state->cpu, TRUE);
  m6502_irq(state->cpu, false);
  m6502_reset(state->cpu);
  
  bool loop = true, step = false;
  int i = 0, ch;
  uint8_t page = 0;
  
  nodelay(stdscr, TRUE);
  clear();
  while(loop) {
    int cycles;
    if((ch = getch()) != ERR) {

      
      if(ch == 'q')
	loop = false;
      else if(ch == ' ')
	step = true;
      else if(ch == KEY_LEFT) {
	if(page > 0)
	  page--;
      }
      else if(ch == KEY_RIGHT) {
	if(page < 0xff)
	  page++;
      }
      else if(ch == KEY_UP) {
	if(page < 0xf0)
	  page = (page & 0xf0) + 0x10;
      }
      else if(ch == KEY_DOWN) {
	if(page > (page & 0xf0))
	  page = page & 0xf0;
	else if (page > 0)
	  page -= 0x10;
      }
      

    }
    i++;
    
    
    if(step_count == 1 || step) {
      
      char new_op_line[10];
      sprintf(new_op_line, "  i %04x", state->cpu->state.pc);
      update_rw_buffer(new_op_line);
      
      exec_cpu_cycle(&state);
      step = false;
    }
    erase();
    display_disassemble(1,80,state);
    display_rw_buffer(1,60);
    display_state(1,1,state);
    display_memory(12,1,state,page);
    

    refresh();
  }

  endwin();
  return 0;
}

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


int init_menu() {
  bool selected = false;
  int select_mode = 0, ch;
  while(selected == false) {
    
    mvprintw(1,1,"6502 Emulator");
    mvprintw(3,1,"Copyright (C) 2019 Patrick Günthard");
    mvprintw(4,1,"This program comes with ABSOLUTELY NO WARRANTY; Press [w] key to This is free ");
    mvprintw(5,1,"software, and you are welcome to redistribute it under certain conditions;");
    mvprintw(6,1,"Press [l] to show license.");
    mvprintw(8,1,"You should have received a copy of the GNU General Public License along with");
    mvprintw(9,1,"this program.  If not, see <https://www.gnu.org/licenses/>.");
    mvprintw(11,1,"Use Arrow Keys to select stepping or continuous mode. Press Enter to continue.");

    if(select_mode == 0)
      mvprintw(12,1,"-> stepping     continuous");
    else
      mvprintw(12,1,"   stepping  -> continuous");
    ch = getch();
    if (ch != ERR) {
      if(ch == KEY_LEFT || ch == KEY_RIGHT)
	select_mode = (select_mode == 0 ? 1 : 0);
      else if(ch == 'w')
        license_warranty_info(warranty_info_text, warranty_info_text_length);
      else if(ch == 'l')
        license_warranty_info(license_info_text, license_info_text_length);
      else if(ch == '\n')
	selected = true;
      clear();
      refresh();
    }

  }
  return select_mode;
}







void license_warranty_info(char text[][80], int length) {
  bool quit = false;
  int position = 0, ch;
  while(quit == false) {
    clear();
    mvprintw(0,0,"Press [q] to close. Press any other key to scroll down");
    for(int x = 0; x < (COLS < 80 ? COLS : 80); x++) {
      mvprintw(1,x,"-");
    }
    for(int y = 2; y < LINES - 2; y++) {
      if(y + position < length)
	mvprintw(y,0,text[y + position]);
    }
    refresh();
    ch = getch();
    if(ch == 'q')
      quit = true;
    else
      position++;
    
  }
}
