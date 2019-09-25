/*
  emulator.c  Emulator for a custom 6502-based Microcomputer
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
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <emulation/CPU/6502.h>

#include "romloader.h"
#include "helper.h"
#include "emulator.h"


zuint8 bus_read(void *context, zuint16 address)
{
  emulator_state *state = (emulator_state*) context;
  if(address == 0xF7FF) {
    uint8_t byte = tapeinterface_read(&state, true);
    state->debug_read(address,byte);
    return byte;
  }
  else {
    state->debug_read(address,state->memory[address]);
    return state->memory[address];
  }
}

void bus_write(void *context, zuint16 address, zuint8 value) {
  emulator_state *state = (emulator_state*) context;
  state->debug_write(address,state->memory[address]);
  state->memory[address] = value;
}


void init_emulator(emulator_state **state, long clockspeed)
{
  
  *state = malloc(sizeof(emulator_state));
  (*state)->memory = (zuint8*) malloc(0x10000 * sizeof(zuint8));

  load_rom("bin/rom.tbl", state);

  (*state)->clockspeed = clockspeed;
  (*state)->cpu = (M6502*) malloc(sizeof(M6502));
  (*state)->cpu->read = &bus_read;
  (*state)->cpu->write = &bus_write;
  (*state)->cpu->context = *state;

  tapeinterface_init(state, "bin/tape.tbl");
}


size_t exec_cpu_cycle(emulator_state **state)
{
  zusize cycles =  m6502_run((*state)->cpu, 1);
  usleep(cycles * (1000000 / (*state)->clockspeed));
  return cycles;
}

void tapeinterface_init(emulator_state **state, char *table_file)
{
  char *table, *tofree, *file;
  readfile(&table, "bin/tape.tbl", false);
  tofree = strdup(table);
  file = strsep(&table, "\n");
  (*state)->hw_state.tape_size = readfile((char**)&(*state)->hw_state.tape_input_buffer, file, true);
  (*state)->hw_state.tape_byte_position = 0;
  (*state)->hw_state.tape_bit_position = 7;
  (*state)->hw_state.tape_byte = 0b00000000;
  (*state)->hw_state.tape_last_state_change = get_timestamp_ms();
  (*state)->hw_state.tape_read_wait = true;
  (*state)->hw_state.tape_bits_per_sec = 4;
  (*state)->hw_state.tape_started = false;
}

uint8_t tapeinterface_read(emulator_state **state, bool change_state)
{
   
   if(change_state && get_timestamp_ms() - (*state)->hw_state.tape_last_state_change
      > (500) / (*state)->hw_state.tape_bits_per_sec) {
     
    (*state)->hw_state.tape_last_state_change = get_timestamp_ms();
    if((*state)->hw_state.tape_byte_position == (*state)->hw_state.tape_size) {
      (*state)->hw_state.tape_byte = 0b00000001;
    }
    else {
      if((*state)->hw_state.tape_read_wait) {
        (*state)->hw_state.tape_read_wait = false;
        (*state)->hw_state.tape_byte = 0b00000000;
      }
      else if((*state)->hw_state.tape_started == true) {
        (*state)->hw_state.tape_read_wait = true;
        uint8_t current_byte = ((*state)->hw_state.tape_input_buffer[(*state)->hw_state.tape_byte_position] >> (*state)->hw_state.tape_bit_position) & 0b00000001;
        (*state)->hw_state.debug = ((*state)->hw_state.debug << 1) + current_byte;
        (*state)->hw_state.tape_byte = 0b00000010 + current_byte;
        
        if((*state)->hw_state.tape_bit_position == 0) {
          (*state)->hw_state.tape_bit_position = 8;
          (*state)->hw_state.tape_byte_position++;
          (*state)->hw_state.debug = 0;
        }
        (*state)->hw_state.tape_bit_position--;
      }
    }
    
  }
  return (*state)->hw_state.tape_byte;
}
