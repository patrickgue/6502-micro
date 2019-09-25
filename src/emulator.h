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


#include <emulation/CPU/6502.h>
#include <stdint.h>
#include <stdio.h>


struct s_hardware_state {
  /* Tape Interface */
  uint8_t tape_byte;
  unsigned long long tape_last_state_change;
  uint8_t *tape_input_buffer;
  uint16_t tape_size;
  uint16_t tape_byte_position;
  uint8_t tape_bit_position;
  bool tape_read_wait;
  int tape_bits_per_sec;
  bool tape_started;
  uint8_t debug;
};

typedef struct s_hardware_state hardware_state;

struct s_emulator_state {
  zuint8 *memory;
  M6502 *cpu;
  long clockspeed;
  void (*debug_read)(uint16_t, uint8_t);
  void (*debug_write)(uint16_t, uint8_t);
  hardware_state hw_state;
};

typedef struct s_emulator_state emulator_state;

void init_emulator(emulator_state**, long);

size_t exec_cpu_cycle(emulator_state **);

void tapeinterface_init(emulator_state**, char *);
  
uint8_t tapeinterface_read(emulator_state **);

#endif
