/*
  assembler.c  Assembler for the 6502 microprocessor
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
#include <stdint.h>
#include <string.h>

#include "assembler.h"
#include "helper.h"

void test()
{
  printf("--- RUN TESTS ---\n");
  printf("--- END TESTS ----\n");
}

label *labels;
int labels_count = 0;
  

int main(int argc, char **argv) {

  if(argc < 3) {
    return 0;
  }
  else {
    welcome("as65");
  }

  char *assembly_code, *code_tf, *line;
  size_t assembly_code_size = readfile(&assembly_code, argv[1], false);

  uint16_t program_counter = 0;
  uint16_t pc_offset = 0;
  bool pc_offset_set = false;
  uint8_t *buffer = (uint8_t*) malloc(0);;

  char *tofree, *op;
  
  labels = malloc(0);
  
  if(assembly_code_size == 0)
    return 1;

  code_tf = strdup(assembly_code);
  while((line = strsep(&assembly_code, "\n")) != NULL) {
    uint8_t *bytes = malloc(3);
    line = trim(line);
    printf("\"%s\"\n", line);
    size_t op_size;
    switch(get_line_type(line)) {
    case operation:
      printf("op: ");
      op_size = construct_binopt(line, &bytes);
    
      printf("(%zu) %02x ", op_size, bytes[0]);
      if(op_size > 1)
	printf(" %02x ", bytes[1]);
      if(op_size > 2)
	printf("%02x", bytes[2]);
      printf("\n");

      program_counter += op_size;
      buffer = realloc(buffer, program_counter);
      for(int i = 0; i < op_size; i++) {
	buffer[program_counter - op_size + i] = bytes[i];
      }
      break;
    case pseudoop:
      tofree = strdup(line);
      op = strsep(&line, " ");
      if(strcmp(op, ".pc") == 0) {
	 program_counter = parse_number(line, absolute);
	 if(!pc_offset_set)
	   pc_offset = program_counter;
      }
      break;
    }
    
  }

  for(int i = pc_offset; i < program_counter; i++) {
    printf("%04x %02x\n", i, buffer[i]);
  }

  FILE *f = fopen(argv[2], "wb");
  fwrite(buffer + pc_offset, 1, program_counter - pc_offset , f);
  fclose(f);
  
  return 0;
}


size_t construct_binopt(char line[64], uint8_t **bytes) {
  char *cmd;
  addressing_information addr_info = {0, 0, 0};
  if(!contains_single(line, ' ')) {
    strcpy(cmd, line);
    for(int i = 0; i < implied_ops_count; i++) {
      if(strcmp(line, implied_ops[i]) == 0)
	addr_info.mode = implied;
    }
    for(int i = 0; i < accum_ops_count; i++) {
      if(strcmp(line, accum_ops[i]) == 0)
	addr_info.mode = accumlator;
    }
  }
  else {
    char *tofree;
    tofree = strdup(line);
    cmd = strsep(&line, " ");
    bool force_word = false;
    for(int i = 0; i < force_word_ops_size; i++)
      if(strcmp(force_word_ops[i], cmd) == 0) force_word = true;
    addr_info = calc_addressing_information(line, force_word);
    for(int i = 0; i < relative_ops_count; i++) {
      if(strcmp(cmd, relative_ops[i]) == 0) {
	addr_info.mode = relative;
      }
    }
  }

  int opindex;
  for(opindex = 0; opindex < 56; opindex++) {
    if(strcmp(opcode_label_table[opindex], cmd) == 0) {
      break;
    }
  }

  
  size_t op_size = op_address_size[addr_info.mode];
  *bytes = malloc(op_size);
  (*bytes)[0] = opcode_table[opindex][addr_info.mode];

  if(op_size > 1) {
    (*bytes)[1] = addr_info.lbyte;
  }
  if(op_size > 2) {
    (*bytes)[2] = addr_info.hbyte;
  }

  return op_size;
}


addressing_information calc_addressing_information(char number[18], bool force_word) {
  addressing_mode mode;
  if(!contains(number, "(),#XY")) {
    uint16_t tmp_nr = parse_number(number, absolute);
    if(tmp_nr > 0xff || force_word)
      mode = absolute;
    else
      mode = zeropage;
  }
  else if(contains(number, "#"))
    mode = immediate;
  else if(contains(number, "()") && !contains_single(number, ','))
    mode = indirect;
  else if(contains(number, "(),") && (contains_single(number, 'X') || contains_single(number, 'Y'))) {
    uint16_t tmp_nr = parse_number(number, indirect_x);
    if(tmp_nr > 0xff || force_word) {
      if(contains_single(number, 'X'))
	mode = indirect_x;
      else if(contains_single(number, 'Y'))
	mode = indirect_y;
    }
    else {
      if(contains_single(number, 'X'))
	mode = zeropage_x;
      else if(contains_single(number, 'Y'))
	mode = zeropage_y;
    }
  }

  uint16_t nbr = parse_number(number, mode);
  uint8_t lb = (nbr & 0x00ff);
  uint8_t hb = (nbr & 0xff00) >> 8;

  addressing_information info = {mode, lb, hb};

  return info;
}


uint16_t parse_number(char number[18], addressing_mode mode) {
  char buffer[18] = "", buffer2[18] = "";
  int i;
  uint16_t parsed_number;
  switch(mode) {
  case absolute:
    strcpy(buffer, number);
    break;
  case absolute_x:
  case absolute_y:
    i=0;
    while(number[i] != ',') {
      buffer[i] = number[i];
      i++;
    }
    buffer[i] = '\0';
    break;
  case immediate:
    i=1;
    while(number[i] != '\0') {
      buffer[i-1] = number[i];
      i++;
    }
    break;
  case indirect:
    i=1;
    do {
      buffer[i-1] = number[i];
      i++;
    } while(number[i] != ')');
    buffer[i] = '\0';
    break;
  case indirect_x:
  case indirect_y:
    i=1;
    do {
      buffer[i-1] = number[i];
      i++;
    } while(number[i] != ',' && number[i] != ')');
    buffer[i] = '\0';
    break;
  case zeropage:
    i = 0;
    while(number[i] != '\0') {
      buffer[i] = number[i];
      i++;
    }
    break;
  case zeropage_x:
  case zeropage_y:
    i = 0;
    do {
      buffer[i] = number[i];
      i++;
    } while(number[i] != ',');
    buffer[i] = '\0';
    break;
  case implied:
  case relative:
  case accumlator:
    break;
  }

  if(buffer[0] == '$') {
    parsed_number = strtol(buffer+1, NULL, 16);
  }
  else if(buffer[0] == '&') {
    parsed_number = strtol(buffer+1, NULL, 8);
  }
  else if(buffer[0] == '%') {
    parsed_number = strtol(buffer+1, NULL, 2);
  }
  else {
    parsed_number = strtol(buffer, NULL, 10);
  }
  
  return parsed_number;
}


line_type get_line_type(char line[64]) {
  if(strlen(line) == 0) {
    return skip;
  }
  for(int i = 0; i < pseudo_ops_size; i++) {
    if(strstr(line, pseudo_ops[i]) != NULL) {
      return pseudoop;
    }
  }
  if(contains_single(line, ':')) {
    return lbl;
  }
  return operation;
}


void add_label(char *line, uint16_t pc) {
  char *ln = malloc(strlen(line));
  strcpy(ln, line);
  labels = realloc(labels, (labels_count+1) * sizeof(label));
  labels[labels_count].pc = pc;
  strcpy(labels[labels_count].labelname, ln);
  free(ln);
  labels_count++;
}


uint8_t * link(uint8_t *memory, label *labels) {
  return memory;
}
