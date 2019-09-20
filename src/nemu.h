#ifndef NEMU_H
#define NEMU_H

int init_menu();

void debug_bus_read(uint16_t, uint8_t);
void debug_bus_write(uint16_t, uint8_t);

void license_warranty_info(char[][80], int);

#endif
