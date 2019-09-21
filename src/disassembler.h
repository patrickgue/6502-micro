#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>

uint8_t disassemble_line(char **output_line, uint8_t *memory, uint32_t pc, bool current);

#endif
