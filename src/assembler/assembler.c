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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "../helper.h"
#include "disassembler.h"

#include "assembler.h"

label *labels;
int labels_count = 0;

label *label_references;
int label_references_count;
  
void usage() {
  printf("Usage:\nas65 [-v] -i [FILE] -o [FILE]\n");
}

int main(int argc, char **argv) {
  int opt;
  bool verbose_flag = false;
  char *assembly_code_filename;
  char *binary_output_filename;

  while ((opt = getopt(argc, argv, "vli:o:")) != -1) {
    switch (opt) {
      case 'l':
        welcome("as65");
        break;
      case 'v':
        verbose_flag = true;
        break;
      case 'i':
        if(optarg == NULL) {
          usage();
          return -1;
        }
        assembly_code_filename = malloc(strlen(optarg) + 1);
        strcpy(assembly_code_filename, optarg);
        break;
      case 'o':
        if(optarg == NULL) {
          usage();
          return -1;
        }
        binary_output_filename = malloc(strlen(optarg) + 1);
        strcpy(binary_output_filename, optarg);
        break;
      default:
        usage();
        break;
    }
  }

  if(assembly_code_filename == NULL || binary_output_filename == NULL) {
    usage();
    return -1;
  }

  
  

  char *assembly_code, *code_tf, *line;
  size_t assembly_code_size = readfile(&assembly_code, assembly_code_filename, false);

  uint32_t program_counter = 0;
  uint16_t pc_offset = 0;
  bool pc_offset_set = false;
  uint8_t *buffer = (uint8_t*) malloc(0);
  uint8_t *bytes = malloc(3);

  char *tofree, *op;
  
  labels = malloc(0);
  label_references = malloc(0);
  
  if(assembly_code_size == 0)
    return 1;

  code_tf = strdup(assembly_code);
  while((line = strsep(&assembly_code, "\n")) != NULL) {
    line = remove_comment(line);
    line = trim(line);
    if(verbose_flag) { printf("\"%s\"\n", line); }
    size_t op_size;
    line_type current_line_type = get_line_type(line);
    switch(current_line_type) {
    case op_with_label:
    case operation:
      if(current_line_type == op_with_label) {
	      strcpy(line, add_label_reference(line, program_counter));
      }

      op_size = construct_binopt(line, &bytes);
    
      if(verbose_flag) {
        printf("(%zu) %02x ", op_size, bytes[0]);
        if(op_size > 1)
          printf(" %02x ", bytes[1]);
        if(op_size > 2)
          printf("%02x", bytes[2]);
        printf("\n");
      }

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
        if(!pc_offset_set) {
          pc_offset = program_counter;
          pc_offset_set = true;
        }
      }
      else if(strcmp(op, ".byte") == 0) {
        buffer[program_counter] = parse_number(line, zeropage) & 0x00ff;
        program_counter++;
      }
      else if(strcmp(op, ".word") == 0) {
        buffer[program_counter] = (parse_number(line, absolute) & 0x00ff);
        buffer[program_counter + 1] = (parse_number(line, absolute) & 0xff00) >> 8;
        program_counter += 2;
	
      }
      break;
    case lbl:
      add_label(line, program_counter);
      break;
    case skip:
      break;
    }
    
  }


  /* inject label references */
  for(int i = 0; i < label_references_count; i++) {
    for(int j = 0; j < labels_count; j++) {
      if(strcmp(label_references[i].labelname, labels[j].labelname) == 0) {
        if(label_references[i].rel) {
          /* signed */ int8_t b = (int8_t) ((labels[j].pc - label_references[i].pc - 2) & 0x00ff);
          buffer[label_references[i].pc + 1] = b;
        }
        else {
          buffer[label_references[i].pc + 1] = (labels[j].pc & 0x00ff);
          buffer[label_references[i].pc + 2] = (labels[j].pc & 0xff00) >> 8;
        }
      }
    }
  }
  if(verbose_flag) {
    printf("Labels:\n");
    for(int i = 0; i < labels_count; i++) {
      printf("%04x %s\n", labels[i].pc, labels[i].labelname);
    }

    printf("\nLabel references:\n");
    for(int i = 0; i < label_references_count; i++) {
      printf("%04x %s %zu %s\n",
      label_references[i].pc,
      label_references[i].labelname,
      label_references[i].size,
      label_references[i].rel ? "rel" : "abs");
    }


    printf("\nProgram (disassembled)\n");
    uint32_t i = pc_offset, pc_add;
    
    do {
      char *line;
      pc_add = disassemble_line(&line, buffer, i, false);
      printf("%s\n", line);
      i += pc_add;
      free(line);
    } while( pc_add != 0 && i <= program_counter);
  }
  FILE *f = fopen(binary_output_filename, "wb");
  fwrite(buffer + pc_offset, 1, program_counter - pc_offset , f);
  fclose(f);


  free(buffer);
  free(labels);
  free(label_references);
  free(bytes);
  return 0;
}


size_t construct_binopt(char line[64], uint8_t **bytes) {
  char *cmd = malloc(4);
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
    bool force_word = is_force_word_op(cmd);
    addr_info = calc_addressing_information(line, force_word);
    if(is_relative_addr_op(cmd)) {
      addr_info.mode = relative;
    }
  }

  int opindex;
  for(opindex = 0; opindex < 56; opindex++) {
    if(strcmp(opcode_label_table[opindex], cmd) == 0) {
      break;
    }
  }

  
  size_t op_size = op_address_size[addr_info.mode];
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
  else if(!contains(number, "()#") && (contains_single(number, 'X') || contains_single(number, 'Y'))) {
    uint16_t tmp_nr = parse_number(number,absolute_x);
    if(tmp_nr > 0xff || force_word) {
      if(contains_single(number, 'X'))
      	mode = absolute_x;
      else if(contains_single(number, 'Y'))
	      mode = absolute_y;
    }
    else {
      if(contains_single(number, 'X'))
	      mode = zeropage_x;
      else if(contains_single(number, 'Y'))
	      mode = zeropage_y;
    }
  }
  else if(contains(number, "#"))
    mode = immediate;
  else if(contains(number, "()") && !contains_single(number, ','))
    mode = indirect;
  else if(contains(number, "(),") && (contains_single(number, 'X') || contains_single(number, 'Y'))) {
    if(contains_single(number, 'X'))
      mode = indirect_x;
    else if(contains_single(number, 'Y'))
      mode = indirect_y;
  
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
  if(contains(line, "[]")) {
    return op_with_label;
  }
  return operation;
}

bool is_implied_addr_op(char *cmd) {
  for(int i = 0; i < implied_ops_count; i++) {
    if(strcmp(cmd, implied_ops[i]) == 0) {
      return true;
    }
  }
  return false;
}

bool is_accum_addr_op(char *cmd) {
  for(int i = 0; i < accum_ops_count; i++) {
    if(strcmp(cmd, accum_ops[i]) == 0) {
      return true;
    }
  }
  return false;
}

bool is_relative_addr_op(char *cmd) {
  for(int i = 0; i < relative_ops_count; i++) {
    if(strcmp(cmd, relative_ops[i]) == 0) {
      return true;
    }
  }
  return false;
}

bool is_force_word_op(char *cmd) {
  for(int i = 0; i < force_word_ops_count; i++) {
    if(strcmp(cmd, force_word_ops[i]) == 0) {
      return true;
    }
  }
  return false;
}

void add_label(char *line, uint16_t pc) {
  char *ln = malloc(strlen(line));
  strcpy(ln, line);
  ln[strlen(ln)-1] = '\0';

  for(int i = 0; i < labels_count; i++) {
    if(strcmp(labels[i].labelname, ln) == 0 && labels[i].pc != pc) {
      printf("Error: Label already defined\n");
      return;
    }
  }
  
  labels = realloc(labels, (labels_count+1) * sizeof(label));
  labels[labels_count].pc = pc;
  strcpy(labels[labels_count].labelname, ln);
  labels_count++;
}

char* add_label_reference(char *line, uint16_t pc) {
  char *ln = malloc(strlen(line));
  char *tofree, *number_str, *output;
  uint16_t address;
  strcpy(ln, line); 
  tofree = strdup(ln);

  char *cmd = strsep(&ln, " ");
  
  
  /* remove brackets */
  ln++;
  ln[strlen(ln)-1] = '\0';

  label_references = realloc(label_references, (++label_references_count) * sizeof(label) );

  strcpy(label_references[label_references_count-1].labelname, ln);

  label_references[label_references_count-1].pc = pc;

  if(is_relative_addr_op(cmd)) {
    label_references[label_references_count-1].rel = true;
    label_references[label_references_count-1].size = 1;
    output = malloc(8);
    sprintf(output, "%s $00", cmd);
  }
  else {
    label_references[label_references_count-1].rel = false;
    label_references[label_references_count-1].size = 2;
    output = malloc(10);
    sprintf(output, "%s $0000", cmd);
  }

  free(tofree);

  return output;
}

char *remove_comment(char *line) {
  if(contains_single(line, ';')) {
    for(int i = 0; i < strlen(line); i++) {
      if(line[i] == ';') {
        line[i] = '\0';
        break;
      }
    }
  }
  return line;
}