/*
  nemu_debug.h  header file for nemu_debug.c 
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
#ifndef NEMU_DEBUG_H
#define NEMU_DEBUG_H

void display_state(int,int,emulator_state*);

void display_memory(int,int,emulator_state *,uint8_t);

void display_disassemble(int,int,emulator_state*);

void update_rw_buffer(char[]);

void display_rw_buffer(int,int);

#define RW_BUFFER_LINE_LEN 11 // rw 1 + space 1 + addr 4 + space 1 + data 2 + zero 1
static char rw_buffer[20][RW_BUFFER_LINE_LEN] = {"","","","","","","","","",""};
static int  rw_buffer_length = 20;


#endif
