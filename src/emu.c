#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <emulation/CPU/6502.h>

#include "emu.h"

#include "helper.h"
#include "emulator.h"

#define CLOCKSPEED  2000000 // 2MHz

emulator_state *state;


void debug_bus_read(uint16_t address, uint8_t data) {
  printf("r 0x%04x 0x%02x\n", address, data);
}

void debug_bus_write(uint16_t address, uint8_t data) {
  printf("w 0x%04x 0x%02x\n", address, data);
}


bool loop = true;

int main(int argc, char **argv)
{
  welcome("emu");
  init_emulator(&state, CLOCKSPEED);

  state->debug_read = &debug_bus_read;
  state->debug_write = &debug_bus_write;

  
  m6502_power(state->cpu, TRUE);
  m6502_irq(state->cpu, false);
  m6502_reset(state->cpu);

  while(loop == true) {
    zusize cycles = exec_cpu_cycle(&state);
    if(argc > 1 && strcmp(argv[1], "-s") == 0) {
      bool wait = true;

      while(wait) {
	char dbstr[64];
	printf("dbg: ");
	fgets(dbstr, 64, stdin);
	wait = debug(state, dbstr);
      }
    }
    else {
     
    }
  }

}



bool debug(emulator_state *state, char str[64]) {
  if(strlen(str) == 0)
    return false;

  char *to_free;
  to_free = strdup(str);

  char *cmd = strsep(&str, " ");
  if(strcmp(cmd, "mem") == 0) {
    zuint16 addr = strtol(str, NULL, 16);
    printf("%02x\n", state->memory[addr]);
    return true;
  }
  else if(strcmp(cmd, "state\n") == 0) {
    printf("Registers\nA: %02x  X: %02x  Y: %02x  PC: %04x \nFlags\n",
	   state->cpu->state.a,
	   state->cpu->state.x,
	   state->cpu->state.y,
	   state->cpu->state.pc);
    printf("| N | V |   | B | D | I | Z | C |\n  %d   %d       %d   %d\
   %d   %d   %d\n",
	   (state->cpu->state.p & 0b10000000) >> 7,
	   (state->cpu->state.p & 0b01000000) >> 6,
	   (state->cpu->state.p & 0b00010000) >> 4,
	   (state->cpu->state.p & 0b00001000) >> 3,
	   (state->cpu->state.p & 0b00000100) >> 2,
	   (state->cpu->state.p & 0b00000010) >> 1,
	   (state->cpu->state.p & 0b00000001));
    return true;
  }
  else if(strcmp(cmd, "irq\n") == 0) {
    m6502_irq(state->cpu, true);
    m6502_irq(state->cpu, true);
    return true;
    }
  else if(strcmp(cmd, "nmi\n") == 0) {
    m6502_nmi(state->cpu);
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
    m6502_reset(state->cpu);
    return false;
  }

  return false;
}
