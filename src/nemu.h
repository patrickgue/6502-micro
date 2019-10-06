#ifndef NEMU_H
#define NEMU_H

#include <stdint.h>

int
init_menu();
void
display_help();

void 
debug_bus_read(uint16_t, uint8_t);

void 
debug_bus_write(uint16_t, uint8_t);

void
license_warranty_info(char[][80], int);

#endif
