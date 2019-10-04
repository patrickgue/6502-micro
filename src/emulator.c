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
  EMU_S->memory = (zuint8*) malloc(0x10000 * sizeof(zuint8));

  load_rom("bin/rom.tbl", state);

  EMU_S->clockspeed = clockspeed;
  EMU_S->cpu = (M6502*) malloc(sizeof(M6502));
  EMU_S->cpu->read = &bus_read;
  EMU_S->cpu->write = &bus_write;
  EMU_S->cpu->context = *state;

  tapeinterface_init(state, "bin/tape.tbl");


  HW_S.video_buffer_bit_pos = 0;
  HW_S.video_buffer_size = 0;

  HW_S.ps2_encoding_table_size = init_ps2_encodings(&(HW_S.ps2_encoding_table));
  
  HW_S.ps2_buffer = malloc(0);
  HW_S.ps2_buffer_position = 0;
  strcpy(HW_S.ps2_debug, "");

}


size_t exec_cpu_cycle(emulator_state **state)
{
  zusize cycles =  m6502_run(EMU_S->cpu, 1);
  usleep(cycles * (1000000 / EMU_S->clockspeed));
  EMU_S->passed_cycles += cycles;
  int ps2_target_clockcycles = EMU_S->clockspeed / 10000;// 10 MHz
  if((EMU_S->passed_cycles - (EMU_S->passed_cycles % 10)) % ps2_target_clockcycles == 0) {
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
  HW_S.tape_size = readfile((char**)&HW_S.tape_input_buffer, file, true);
  HW_S.tape_byte_position = 0;
  HW_S.tape_bit_position = 7;
  HW_S.tape_byte = 0b00000000;
  HW_S.tape_last_state_change = get_timestamp_ms();
  HW_S.tape_read_wait = true;
  HW_S.tape_bits_per_sec = 4;
  HW_S.tape_started = false;
}

uint8_t tapeinterface_read(emulator_state **state, bool change_state)
{
   
   if(change_state && get_timestamp_ms() - HW_S.tape_last_state_change
      > (500) / HW_S.tape_bits_per_sec) {
     
    HW_S.tape_last_state_change = get_timestamp_ms();
    if(HW_S.tape_byte_position == HW_S.tape_size) {
      HW_S.tape_byte = 0b00000001;
    }
    else {
      if(HW_S.tape_read_wait) {
        HW_S.tape_read_wait = false;
        HW_S.tape_byte = 0b00000000;
      }
      else if(HW_S.tape_started == true) {
        HW_S.tape_read_wait = true;
        uint8_t current_byte = (HW_S.tape_input_buffer[HW_S.tape_byte_position] >> HW_S.tape_bit_position) & 0b00000001;
        HW_S.tape_byte = 0b00000010 + current_byte;
        
        if(HW_S.tape_bit_position == 0) {
          HW_S.tape_bit_position = 8;
          HW_S.tape_byte_position++;
        }
        HW_S.tape_bit_position--;
      }
    }
    
  }
  return HW_S.tape_byte;
}


void vt100_add_bit(emulator_state **state, uint8_t data) {
  HW_S.video_buffer[HW_S.video_buffer_size] = HW_S.video_buffer[HW_S.video_buffer_size] << 1;
  HW_S.video_buffer[HW_S.video_buffer_size] += data & 0b00000001;
  
  if(HW_S.video_buffer_bit_pos == 7) {
    HW_S.video_buffer_bit_pos = 0;
    HW_S.video_buffer_size++;
  }
  else {
    HW_S.video_buffer_bit_pos++;
  }
}

void ps2_send_bit(emulator_state **state) {
  if(HW_S.ps2_skip_for_next == true) {
    HW_S.ps2_skip_for_next = false;
    HW_S.ps2_current_buffer_bit = false;
  }
  else {
    if(HW_S.ps2_buffer_position != 0) {
      if(HW_S.ps2_buffer_bit_position < 8) {
        HW_S.ps2_current_buffer_bit = (HW_S.ps2_buffer[0] & (0b10000000 >> HW_S.ps2_buffer_bit_position)) > 0 ? true : false;
        HW_S.ps2_buffer_bit_position++;
      }
      else if(HW_S.ps2_buffer_bit_position == 8) {
        HW_S.ps2_current_buffer_bit = true; //parity bit (not calculated)
        memcpy(HW_S.ps2_buffer, HW_S.ps2_buffer +1, --HW_S.ps2_buffer_position);
        HW_S.ps2_buffer_bit_position = 0;
        HW_S.ps2_skip_for_next = true;
      }
      sprintf(HW_S.ps2_debug, "%s%d", HW_S.ps2_debug, HW_S.ps2_current_buffer_bit);
      m6502_nmi(EMU_S->cpu);
    }
  }
}

void ps2_add_char_to_buffer(emulator_state **state, char *ncurses_char_sequence) {
  uint8_t * new_sequence;
  int new_sequence_length = encode_ps2(HW_S.ps2_encoding_table, HW_S.ps2_encoding_table_size, ncurses_char_sequence, &new_sequence);
  
  HW_S.ps2_buffer = realloc(HW_S.ps2_buffer, HW_S.ps2_buffer_position + new_sequence_length);
  memcpy(HW_S.ps2_buffer + HW_S.ps2_buffer_position, new_sequence, new_sequence_length);
  HW_S.ps2_buffer_position += new_sequence_length;
}
