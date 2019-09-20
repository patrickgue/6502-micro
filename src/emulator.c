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
  state->debug_read(address,state->memory[address]);
  return state->memory[address];
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

  load_rom("bin/loadrom.tbl",(*state)->memory);

  (*state)->clockspeed = clockspeed;
  (*state)->cpu = (M6502*) malloc(sizeof(M6502));
  (*state)->cpu->read = &bus_read;
  (*state)->cpu->write = &bus_write;
  (*state)->cpu->context = *state;
}


size_t exec_cpu_cycle(emulator_state **state) {
  zusize cycles =  m6502_run((*state)->cpu, 1);
  usleep(cycles * (1000000 / (*state)->clockspeed));
  return cycles;
}
