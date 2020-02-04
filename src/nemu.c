#include <stdbool.h>
#include <stdio.h>

#include <ncurses.h>

#include "emulator.h"
#include "license.h"
#include "nemu_debug.h"

#include "nemu.h"

#define CLOCKSPEED 2000000

int
main(int argc, char** argv)
{

  /* init ncurses */
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  bool debug_interaction_mode = true;
  int CWIDTH = COLS > 120 ? 120 : COLS;
  int CHEIGHT = LINES > 40 ? 40 : LINES;

  /* show init menu */
  int step_count = init_menu();

  /* init_emulator */
  emulator_state* state;
  init_emulator(&state, CLOCKSPEED);
  state->debug_read = &debug_bus_read;
  state->debug_write = &debug_bus_write;
  m6502_power(state->cpu, TRUE);
  m6502_irq(state->cpu, false);
  m6502_reset(state->cpu);

  bool loop = true, step = false, help = false;
  int i = 0, ch;
  uint8_t page = 0;

  if (step_count == 1)
    nodelay(stdscr, TRUE);
  else
    nodelay(stdscr, FALSE);

  clear();
  while (loop) {
    if (step_count == 1 || step) {
      rw_log log = { true, false, state->cpu->state.pc, 0 };
      update_rw_buffer(log);

      exec_cpu_cycle(&state);
      step = false;
    }
    erase();
    attron(A_UNDERLINE);
    mvprintw(1, (CWIDTH / 2) - 14, "6502 Microcomputer Emulator");
    attroff(A_UNDERLINE);
    if (debug_interaction_mode) {
      mvprintw(2, (CWIDTH / 2) - 13, "press [h] to toggle help");

      if (!help) {
        attron(A_UNDERLINE);
        attron(A_REVERSE);
        mvprintw(3, 1, "Debugger");
        attroff(A_REVERSE);
        attroff(A_UNDERLINE);
        display_disassemble(5, 80, state);
        display_rw_buffer(5, 60);
        display_state(5, 1, state);
        display_memory(13, 1, state, page);
        display_tapeinterface(28, 60, state);
        display_ps2(34, 60, state);
      } else {
        display_help(3, 1);
      }
    } else {
      mvprintw(
        2, (CWIDTH / 2) - 22, "press [tab] to switch back to the debugger");
      attron(A_UNDERLINE);
      attron(A_REVERSE);
      mvprintw(3, 1, "IO");
      attroff(A_REVERSE);
      attroff(A_UNDERLINE);

      display_vt100(5, 1, state);
    }
    if ((ch = getch()) != ERR) {
      if (ch == '\t') {
        debug_interaction_mode = debug_interaction_mode ? false : true;
      } else if (debug_interaction_mode) {
        if (ch == 'q')
          loop = false;
        else if (ch == ' ')
          step = true;
        else if (ch == 's') {
          step_count = 0;
          nodelay(stdscr, false);
        } else if (ch == 'c') {
          step_count = 1;
          nodelay(stdscr, true);
        } else if (ch == 'h') {
          help = help ? false : true;
        } else if (ch == KEY_LEFT) {
          if (page > 0)
            page--;
        } else if (ch == KEY_RIGHT) {
          if (page < 0xff)
            page++;
        } else if (ch == KEY_UP) {
          if (page < 0xf0)
            page = (page & 0xf0) + 0x10;
        } else if (ch == KEY_DOWN) {
          if (page > (page & 0xf0))
            page = page & 0xf0;
          else if (page > 0)
            page -= 0x10;
        } else if (ch == 't') {
          state->hw_state.tape_started = true;
        } else if (ch == 'r') {
          state->hw_state.tape_started = false;
          state->hw_state.tape_byte_position = 0;
          state->hw_state.tape_bit_position = 0;
        }
      } else {
        ps2_add_char_to_buffer(&state, ch, keyname(ch));
      }
    }
    i++;
    refresh();
  }

  endwin();
  return 0;
}

int
init_menu()
{
  bool selected = false;
  int select_mode = 0, ch;
  while (selected == false) {

    mvprintw(1, 1, "6502 Emulator");
    mvprintw(3, 1, "Copyright (C) 2019 Patrick Günthard");
    mvprintw(4, 1,
             "This program comes with ABSOLUTELY NO WARRANTY; Press [w] key to "
             "show warranty. ");
    mvprintw(5, 1,
             "This is free software, and you are welcome to redistribute it "
             "under certain");
    mvprintw(6, 1, " conditions; Press [l] to show license.");
    mvprintw(8, 1,
             "You should have received a copy of the GNU General Public "
             "License along with");
    mvprintw(9, 1, 
             "this program.  If not, see <https://www.gnu.org/licenses/>.");
    mvprintw(11, 1,
             "Use Arrow Keys to select stepping or continuous mode. Press "
             "Enter to continue.");

    if (select_mode == 0)
      mvprintw(12, 1, "-> stepping     continuous");
    else
      mvprintw(12, 1, "   stepping  -> continuous");
    ch = getch();
    if (ch != ERR) {
      if (ch == KEY_LEFT || ch == KEY_RIGHT)
        select_mode = (select_mode == 0 ? 1 : 0);
      else if (ch == 'w')
        license_warranty_info(warranty_info_text, warranty_info_text_length);
      else if (ch == 'l')
        license_warranty_info(license_info_text, license_info_text_length);
      else if (ch == '\n')
        selected = true;
      clear();
      refresh();
    }
  }
  return select_mode;
}

void
display_help()
{
  attron(A_UNDERLINE);
  attron(A_REVERSE);
  mvprintw(3, 1, "Help");
  attroff(A_REVERSE);
  attroff(A_UNDERLINE);

  attron(A_UNDERLINE);
  mvprintw(5, 1, "Modes");
  attroff(A_UNDERLINE);

  mvprintw(6, 1, "[tab]\t\tSwitch between Terminal Emulation and Debugger");

  attron(A_UNDERLINE);
  mvprintw(8, 1, "Debugger Functionality");
  attroff(A_UNDERLINE);

  mvprintw(9, 1, "[q]\t\tQuit Emulator");
  mvprintw(10, 1, "[s]\t\tSwitch to stepping mode");
  mvprintw(11, 1, "[c]\t\tSwitch to continous mode");
  mvprintw(12, 1, "[space]\tStep over (only in stepping mode)");

  mvprintw(13, 1, "[<-]\t\tShow previous memory page");
  mvprintw(14, 1, "[->]\t\tShow next memory page");
  mvprintw(15, 1, "[/\\]\t\tSkip to next page group");
  mvprintw(16, 1, "[\\/]\t\tSkip to previous page group");

  attron(A_UNDERLINE);
  mvprintw(18, 1, "Hardware");
  attroff(A_UNDERLINE);

  mvprintw(19, 1, "[t]\t\tStart Tape");
  mvprintw(20, 1, "[r]\t\tRewind Tape");
}

void
license_warranty_info(char text[][80], int length)
{
  bool quit = false;
  int position = 0, ch;
  while (quit == false) {
    clear();
    mvprintw(0, 0, "Press [q] to close. Press any other key to scroll down");
    for (int x = 0; x < (COLS < 80 ? COLS : 80); x++) {
      mvprintw(1, x, "-");
    }
    for (int y = 2; y < LINES - 2; y++) {
      if (y + position < length)
        mvprintw(y, 0, text[y + position]);
    }
    refresh();
    ch = getch();
    if (ch == 'q')
      quit = true;
    else
      position++;
  }
}
