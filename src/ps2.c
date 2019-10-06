#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ps2.h"

/* yes, this is a mess. Must be cleaned up at some point and extended to use other layout than en_US */
int init_ps2_encodings(ps2_encoding **encoding) {
    (*encoding) = malloc(0x80 * sizeof(ps2_encoding));
    int index = 0;
    add_ps2_encoding(encoding, index++, '\n', get_scan_code("Enter"), NONE);
    add_ps2_encoding(encoding, index++, 0x08, get_scan_code("Backspace"), NONE);
    add_ps2_encoding(encoding, index++, '\t', get_scan_code("Tab"), NONE);


    add_ps2_encoding(encoding, index++, ' ', get_scan_code("Spacebar"), NONE);
    add_ps2_encoding(encoding, index++, '!', get_scan_code("1"), SHIFT);
    add_ps2_encoding(encoding, index++, '"', get_scan_code("'"), SHIFT);
    add_ps2_encoding(encoding, index++, '#', get_scan_code("3"), SHIFT);
    add_ps2_encoding(encoding, index++, '$', get_scan_code("4"), SHIFT);
    add_ps2_encoding(encoding, index++, '%', get_scan_code("5"), SHIFT);
    add_ps2_encoding(encoding, index++, '&', get_scan_code("7"), SHIFT);
    add_ps2_encoding(encoding, index++, '\'', get_scan_code("\\"), NONE);
    add_ps2_encoding(encoding, index++, '(', get_scan_code("9"), SHIFT);
    add_ps2_encoding(encoding, index++, ')', get_scan_code("0"), SHIFT);
    add_ps2_encoding(encoding, index++, '*', get_scan_code("3"), SHIFT);
    add_ps2_encoding(encoding, index++, '+', get_scan_code("+"), NONE);
    add_ps2_encoding(encoding, index++, ',', get_scan_code(","), NONE);
    add_ps2_encoding(encoding, index++, '-', get_scan_code("-"), NONE);
    add_ps2_encoding(encoding, index++, '.', get_scan_code("."), NONE);
    add_ps2_encoding(encoding, index++, '/', get_scan_code("/"), NONE);
    for(char i = '0' ; i <= '9'; i++) {
        char str[2] = {i, '\0'}; 
        add_ps2_encoding(encoding, index++, i, get_scan_code(str), NONE);
    }
    add_ps2_encoding(encoding, index++, ':', get_scan_code(";"), SHIFT);
    add_ps2_encoding(encoding, index++, ';', get_scan_code(";"), NONE);
    add_ps2_encoding(encoding, index++, '<', get_scan_code(","), SHIFT);
    add_ps2_encoding(encoding, index++, '=', get_scan_code("="), NONE);
    add_ps2_encoding(encoding, index++, '>', get_scan_code("."), SHIFT);
    add_ps2_encoding(encoding, index++, '?', get_scan_code("/"), SHIFT);
    add_ps2_encoding(encoding, index++, '@', get_scan_code("2"), SHIFT);
    for(char i = 'A'; i <= 'Z'; i++) {
        char str[2] = {i, '\0'}; 
        add_ps2_encoding(encoding, index++, i, get_scan_code(str), SHIFT);
    }
    add_ps2_encoding(encoding, index++, '[', get_scan_code("["), NONE);
    add_ps2_encoding(encoding, index++, '\\', get_scan_code("\\"), NONE);
    add_ps2_encoding(encoding, index++, ']', get_scan_code("]"), NONE);
    add_ps2_encoding(encoding, index++, '^', get_scan_code("6"), SHIFT);
    add_ps2_encoding(encoding, index++, '_', get_scan_code("-"), SHIFT);
    add_ps2_encoding(encoding, index++, '`', get_scan_code("`"), NONE);
    for(char i = 'a'; i <= 'z'; i++) {
        char str[2] = {i-0x20, '\0'}; 
        add_ps2_encoding(encoding, index++, i, get_scan_code(str), NONE);
    }
    add_ps2_encoding(encoding, index++, '{', get_scan_code("["), SHIFT);
    add_ps2_encoding(encoding, index++, '|', get_scan_code("\\"), SHIFT);
    add_ps2_encoding(encoding, index++, '}', get_scan_code("]"), SHIFT);
    add_ps2_encoding(encoding, index++, '~', get_scan_code("`"), SHIFT);
    return index;
}

void add_ps2_encoding(ps2_encoding** encoding, int index, char ascii, ps2_scancode code, ps2_modifier modifier) {
    (*encoding)[index].ascii = ascii;
    if(modifier == NONE) {
        (*encoding)[index].sequence_size = 3;
    }
    else {
        (*encoding)[index].sequence_size = 6;
    }

    (*encoding)[index].sequence = malloc((*encoding)[index].sequence_size * sizeof(uint8_t));

    if(modifier == SHIFT) {
        ps2_scancode shift_mod_code = get_scan_code("Shift (Left)");
        (*encoding)[index].sequence[0] = shift_mod_code.make_code;
        (*encoding)[index].sequence[1] = code.make_code;
        (*encoding)[index].sequence[2] = (code.break_code & 0xff00) >> 8;
        (*encoding)[index].sequence[3] = code.break_code & 0x00ff;
        (*encoding)[index].sequence[4] = (shift_mod_code.break_code & 0xff00) >> 8;
        (*encoding)[index].sequence[5] = shift_mod_code.break_code & 0x00ff;
    }
    else {
        (*encoding)[index].sequence[0] = code.make_code;
        (*encoding)[index].sequence[1] = ((code.break_code & 0xff00) >> 8);
        (*encoding)[index].sequence[2] = code.break_code & 0x00ff;
    }

}

ps2_scancode get_scan_code(char* str) {
    for(int i = 0; i < scancodes_size; i++) {
        if(strcmp(str, scancodes[i].name) == 0) {
            return scancodes[i];
        }
    }
    return scancodes[0];
}

size_t encode_ps2(ps2_encoding *encoding, int encoding_table_size, char *ncurses_str, uint8_t **target) {
    char ascii;
    bool useCtrl = false;
    if(strlen(ncurses_str) == 1) {
        ascii = ncurses_str[0];
    }
    if(strlen(ncurses_str) == 2 && ncurses_str[0] == '^') {
        ascii = ncurses_str[1] + 0x20;
        useCtrl = true;
    }
    for(int i = 0; i < encoding_table_size; i++) {
        if(encoding[i].ascii == ascii) {
            int size_actual = encoding[i].sequence_size + (useCtrl ? 3 : 0);
            *target = malloc(size_actual * sizeof(uint8_t));
            if(useCtrl) {
                memcpy(*target + 1, encoding[i].sequence, encoding[i].sequence_size);
                ps2_scancode ctrl_mod_code = get_scan_code("Ctrl (Left)");
                (*target)[0] = ctrl_mod_code.make_code;
                (*target)[encoding[i].sequence_size + 1] =     (ctrl_mod_code.break_code & 0xff00) >> 8;
                (*target)[encoding[i].sequence_size + 2] = (ctrl_mod_code.break_code & 0x00ff);
            }
            else {
                memcpy(*target, encoding[i].sequence, size_actual);
            }
            return size_actual;
        }
    }
    return 0;
}

char decode_ps2(ps2_encoding *encoding, uint32_t *source, size_t size) {
  return 0;
}