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

#define CLOCKSPEED  2000000 // 2MHz

zuint8 *memory; 
M6502 *cpu;
bool loop = true;


zuint8 bus_read(void *context, zuint16 address)
{
  printf("r 0x%04x 0x%02x\n", address, memory[address]);
  return memory[address];
}

void bus_write(void *context, zuint16 address, zuint8 value) {
  memory[address] = value;
  printf("w 0x%04x 0x%02x\n", address, value);
}


int main(int argc, char **argv)
{
  welcome("emulator");
  memory = (zuint8*) malloc(256 * 256 * sizeof(zuint8));

  load_rom("bin/loadrom.tbl",memory);
  
  cpu = (M6502*) malloc(sizeof(M6502));
  cpu->read = &bus_read;
  cpu->write = &bus_write;
  m6502_power(cpu, TRUE);
  m6502_irq(cpu, false);
  m6502_reset(cpu);

  while(loop == true) {
    zusize cycles =  m6502_run(cpu, 1);
    if(argc > 1 && strcmp(argv[1], "-s") == 0) {
      bool wait = true;

      while(wait) {
	char dbstr[64];
	printf("dbg: ");
	fgets(dbstr, 64, stdin);
	wait = debug(dbstr);
      }
    }
    else {
      usleep(cycles * (1000000 / CLOCKSPEED));
    }
  }

  printf("0x%02x\n", memory[0x0800]);
  
  return 0;
}


bool debug(char str[64]) {
  if(strlen(str) == 0)
    return false;

  char *to_free;
  to_free = strdup(str);

  char *cmd = strsep(&str, " ");
  if(strcmp(cmd, "mem") == 0) {
    zuint16 addr = strtol(str, NULL, 16);
    printf("%02x\n", memory[addr]);
    return true;
  }
  else if(strcmp(cmd, "state\n") == 0) {
    printf("Registers\nA: %02x  X: %02x  Y: %02x  PC: %04x \nFlags\n",
	   cpu->state.a, cpu->state.x, cpu->state.y, cpu->state.pc);
    printf("| N | V |   | B | D | I | Z | C |\n  %d   %d       %d   %d\
   %d   %d   %d\n",
	   (cpu->state.p & 0b10000000) >> 7,
	   (cpu->state.p & 0b01000000) >> 6,
	   (cpu->state.p & 0b00010000) >> 4,
	   (cpu->state.p & 0b00001000) >> 3,
	   (cpu->state.p & 0b00000100) >> 2,
	   (cpu->state.p & 0b00000010) >> 1,
	   (cpu->state.p & 0b00000001));
    return true;
  }
  else if(strcmp(cmd, "irq\n") == 0) {
    m6502_irq(cpu, true);
    m6502_irq(cpu, true);
    return true;
    }
  else if(strcmp(cmd, "nmi\n") == 0) {
    m6502_nmi(cpu);
    return true;
  }
  else if(strcmp(cmd, "n\n") == 0) {
    return false;
  }
  else if(strcmp(cmd, "halt\n") == 0) {
    loop = false;
    return false;
  }
  else if(strcmp(cmd, "reset\n") == 0) {
    m6502_reset(cpu);
    return false;
  }

  return false;
}
