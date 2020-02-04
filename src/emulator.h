/*
  emulator.h  header file for emulator.c
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

#ifndef EMULATOR_H
#define EMULATOR_H

#include "ps2.h"
#include <emulation/CPU/6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct s_hardware_state
{
  /* Tape Interface */
  uint8_t tape_byte;
  unsigned long tape_last_state_change;
  uint8_t* tape_input_buffer;
  uint16_t tape_size;
  uint16_t tape_byte_position;
  uint8_t tape_bit_position;
  bool tape_read_wait;
  int tape_bits_per_sec;
  bool tape_started;

  /* Video (VT100ish) Emulation */
  int video_buffer[1001];
  int video_buffer_size;
  int video_buffer_bit_pos;

  /* PS/2 Keyboard Emulation */
  uint8_t* ps2_buffer;
  int ps2_buffer_position;
  int ps2_buffer_bit_position;
  bool ps2_current_buffer_bit;
  ps2_encoding* ps2_encoding_table;
  int ps2_encoding_table_size;
  bool ps2_skip_for_next;
};

typedef struct s_hardware_state hardware_state;

struct s_emulator_state
{
  zuint8* memory;
  M6502* cpu;
  long clockspeed;
  void (*debug_read)(uint16_t, uint8_t);
  void (*debug_write)(uint16_t, uint8_t);
  unsigned long int passed_cycles;
  hardware_state hw_state;
};

typedef struct s_emulator_state emulator_state;

/* shortcuts to make the code more readable */
#define EMU_S (*state)
#define HW_S EMU_S->hw_state

void
init_emulator(emulator_state**, long);

size_t
exec_cpu_cycle(emulator_state**);

void
tapeinterface_init(emulator_state**, char*);

uint8_t
tapeinterface_read(emulator_state**, bool);

void
vt100_add_bit(emulator_state**, uint8_t);

void
ps2_send_bit(emulator_state**);

void
ps2_add_char_to_buffer(emulator_state**, int, char*);

#endif
