#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>

#include "emulator.h"
#include "license.h"
#include "nemu_debug.h"

#include "nemu.h"

#define CLOCKSPEED 2000000



int main(int argc, char **argv)
{
  
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
  
  if(step_count == 1)
    nodelay(stdscr, TRUE);
  else
    nodelay(stdscr, FALSE);
  
  clear();
  while(loop) {
    int cycles;
    
    if(step_count == 1 || step) {
      rw_log log = {true, false, state->cpu->state.pc, 0};
      update_rw_buffer(log);
      
      exec_cpu_cycle(&state);
      step = false;
    }
    erase();
    display_disassemble(1,80,state);
    display_rw_buffer(1,60);
    display_state(1,1,state);
    display_memory(8,1,state,page);
    display_tapeinterface(22, 60, state);


    if((ch = getch()) != ERR) {

      
      if(ch == 'q')
	      loop = false;
      else if(ch == ' ')
	      step = true;
      else if(ch == 's') {
	      step_count = 0;
	      nodelay(stdscr, false);
      }
      else if(ch == 'c') {
	      step_count = 1;
	      nodelay(stdscr, true);
      }
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
      else if(ch == 't') {
	      state->hw_state.tape_started = true;
      }
      else if(ch == 'r') {
        state->hw_state.tape_started = false;
        state->hw_state.tape_byte_position = 0;
        state->hw_state.tape_bit_position = 0;
      }
    }
    i++;
    
    refresh();
  }

  endwin();
  return 0;
}

int init_menu() {
  bool selected = false;
  int select_mode = 0, ch;
  while(selected == false) {
    
    mvprintw(1,1,"6502 Emulator");
    mvprintw(3,1,"Copyright (C) 2019 Patrick Günthard");
    mvprintw(4,1,"This program comes with ABSOLUTELY NO WARRANTY; Press [w] key to show warranty. ");
    mvprintw(5,1,"This is free software, and you are welcome to redistribute it under certain");
    mvprintw(6,1," conditions; Press [l] to show license.");
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
