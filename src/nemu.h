#ifndef NEMU_H
#define NEMU_H

void license_warranty_info(char[][80], int);

int init_menu();


void debug_bus_read(uint16_t, uint8_t);
void debug_bus_write(uint16_t, uint8_t);

void update_rw_buffer(char[]);
void display_rw_buffer(int,int);

void display_state(int,int,emulator_state*);

void display_memory(int,int,emulator_state *,uint8_t);

void display_disassemble(int,int,emulator_state*);

#endif
