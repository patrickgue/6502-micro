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
#include "ps2.h"
#include "emulator.h"


zuint8 bus_read(void *context, zuint16 address)
{
  emulator_state *state = (emulator_state*) context;
  if(address == 0xF7FF) {
    uint8_t byte = tapeinterface_read(&state, true);
    state->debug_read(address,byte);
    return byte;
  }
  if(address == 0xF7FD) {
    uint8_t byte = state->hw_state.ps2_current_buffer_bit + 0x80;
    state->debug_read(address, byte);
    return byte;
  }
  else {
    state->debug_read(address,state->memory[address]);
    return state->memory[address];
  }
}

void bus_write(void *context, zuint16 address, zuint8 value) {
  emulator_state *state = (emulator_state*) context;
  if(address < 0xf7fe) {
    state->debug_write(address,state->memory[address]);
    state->memory[address] = value;
  }
  else if(address == 0xf7fe) {
    vt100_add_bit(&state, value);
  }
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


  (*state)->hw_state.video_buffer_bit_pos = 0;
  (*state)->hw_state.video_buffer_size = 0;

  (*state)->hw_state.ps2_encoding_table_size = init_ps2_encodings(&((*state)->hw_state.ps2_encoding_table));
  (*state)->hw_state.ps2_buffer = malloc(0);

}


size_t exec_cpu_cycle(emulator_state **state)
{
  zusize cycles =  m6502_run((*state)->cpu, 1);
  usleep(cycles * (1000000 / (*state)->clockspeed));
  (*state)->passed_cycles += cycles;
  int ps2_target_clockcycles = (*state)->clockspeed / 10000;// 10 MHz
  if(((*state)->passed_cycles - ((*state)->passed_cycles % 10)) % ps2_target_clockcycles == 0) {
    ps2_send_bit(state);
  }
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
        (*state)->hw_state.tape_byte = 0b00000010 + current_byte;
        
        if((*state)->hw_state.tape_bit_position == 0) {
          (*state)->hw_state.tape_bit_position = 8;
          (*state)->hw_state.tape_byte_position++;
        }
        (*state)->hw_state.tape_bit_position--;
      }
    }
    
  }
  return (*state)->hw_state.tape_byte;
}


void vt100_add_bit(emulator_state **state, uint8_t data) {
  (*state)->hw_state.video_buffer[(*state)->hw_state.video_buffer_size] = (*state)->hw_state.video_buffer[(*state)->hw_state.video_buffer_size] << 1;
  (*state)->hw_state.video_buffer[(*state)->hw_state.video_buffer_size] += data & 0b00000001;
  
  if((*state)->hw_state.video_buffer_bit_pos == 7) {
    (*state)->hw_state.video_buffer_bit_pos = 0;
    (*state)->hw_state.video_buffer_size++;
  }
  else {
    (*state)->hw_state.video_buffer_bit_pos++;
  }
}

void ps2_send_bit(emulator_state **state) {
  if((*state)->hw_state.ps2_skip_for_next == false) {
    (*state)->hw_state.ps2_skip_for_next = true;
  }
  else {
    if((*state)->hw_state.ps2_buffer_position != 0) {
      if((*state)->hw_state.ps2_buffer_bit_position < 8) {
        (*state)->hw_state.ps2_current_buffer_bit = (*state)->hw_state.ps2_buffer[(*state)->hw_state.ps2_buffer_position] & (0b00000001 << (*state)->hw_state.ps2_buffer_bit_position++);
      }
      else if((*state)->hw_state.ps2_buffer_bit_position == 8) {
        (*state)->hw_state.ps2_current_buffer_bit = true; //parity bit
        memcpy((*state)->hw_state.ps2_buffer, (*state)->hw_state.ps2_buffer +1, --(*state)->hw_state.ps2_buffer_position);
        (*state)->hw_state.ps2_buffer_bit_position = 0;
      }
      m6502_nmi((*state)->cpu);
    }
  }
}

void ps2_add_char_to_buffer(emulator_state **state, char *ncurses_char_sequence) {
  uint8_t * new_sequence;
  int new_sequence_length = encode_ps2((*state)->hw_state.ps2_encoding_table, (*state)->hw_state.ps2_encoding_table_size, ncurses_char_sequence, &new_sequence);
  
  (*state)->hw_state.ps2_buffer = realloc((*state)->hw_state.ps2_buffer, (*state)->hw_state.ps2_buffer_position + new_sequence_length);
  memcpy((*state)->hw_state.ps2_buffer + (*state)->hw_state.ps2_buffer_position, new_sequence, new_sequence_length);
  (*state)->hw_state.ps2_buffer_position += new_sequence_length;
}
