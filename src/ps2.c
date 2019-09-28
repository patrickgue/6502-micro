#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ps2.h"

/* yes, this is a mess. Must be cleaned up at some point and extended to use other layout than en_US */
int init_encodings(ps2_encoding **encoding) {
    (*encoding) = malloc(0x80 * sizeof(ps2_encoding));
    int index = 0;
    add_ps2_encoding(encoding, index, '\n', get_scan_code("Enter"), NONE); index++;
    add_ps2_encoding(encoding, index, 0x08, get_scan_code("Backspace"), NONE); index++;
    add_ps2_encoding(encoding, index, '\t', get_scan_code("Tab"), NONE); index++;


    add_ps2_encoding(encoding, index, ' ', get_scan_code("Spacebar"), NONE); index++;
    add_ps2_encoding(encoding, index, '!', get_scan_code("1"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '"', get_scan_code("'"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '#', get_scan_code("3"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '$', get_scan_code("4"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '%', get_scan_code("5"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '&', get_scan_code("7"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '\'', get_scan_code("\\"), NONE); index++;
    add_ps2_encoding(encoding, index, '(', get_scan_code("9"), SHIFT); index++;
    add_ps2_encoding(encoding, index, ')', get_scan_code("0"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '*', get_scan_code("3"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '+', get_scan_code("+"), NONE); index++;
    add_ps2_encoding(encoding, index, ',', get_scan_code(","), NONE); index++;
    add_ps2_encoding(encoding, index, '-', get_scan_code("-"), NONE); index++;
    add_ps2_encoding(encoding, index, '.', get_scan_code("."), NONE); index++;
    add_ps2_encoding(encoding, index, '/', get_scan_code("/"), NONE); index++;
    for(char i = '0' ; i <= '9'; i++) {
        char str[2] = {i, '\0'}; 
        add_ps2_encoding(encoding, index, i, get_scan_code(str), NONE); index++;
    }
    add_ps2_encoding(encoding, index, ':', get_scan_code(";"), SHIFT); index++;
    add_ps2_encoding(encoding, index, ';', get_scan_code(";"), NONE); index++;
    add_ps2_encoding(encoding, index, '<', get_scan_code(","), SHIFT); index++;
    add_ps2_encoding(encoding, index, '=', get_scan_code("="), NONE); index++;
    add_ps2_encoding(encoding, index, '>', get_scan_code("."), SHIFT); index++;
    add_ps2_encoding(encoding, index, '?', get_scan_code("/"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '@', get_scan_code("2"), SHIFT); index++;
    for(char i = 'A'; i <= 'Z'; i++) {
        char str[2] = {i, '\0'}; 
        add_ps2_encoding(encoding, index, i, get_scan_code(str), SHIFT); index++;
    }
    add_ps2_encoding(encoding, index, '[', get_scan_code("["), NONE); index++;
    add_ps2_encoding(encoding, index, '\\', get_scan_code("\\"), NONE); index++;
    add_ps2_encoding(encoding, index, ']', get_scan_code("]"), NONE); index++;
    add_ps2_encoding(encoding, index, '^', get_scan_code("6"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '_', get_scan_code("-"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '`', get_scan_code("`"), NONE); index++;
    for(char i = 'a'; i <= 'z'; i++) {
        char str[2] = {i+0x20, '\0'}; 
        add_ps2_encoding(encoding, index, i, get_scan_code(str), NONE); index++;
    }
    add_ps2_encoding(encoding, index, '{', get_scan_code("["), SHIFT); index++;
    add_ps2_encoding(encoding, index, '|', get_scan_code("\\"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '}', get_scan_code("]"), SHIFT); index++;
    add_ps2_encoding(encoding, index, '~', get_scan_code("`"), SHIFT); index++;
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
        ps2_scancode modifier_code = get_scan_code("Shift (Left)");
        (*encoding)[index].sequence[0] = modifier_code.make_code;
        (*encoding)[index].sequence[4] = (modifier_code.break_code & 0xff00) >> 8;
        (*encoding)[index].sequence[5] = modifier_code.break_code & 0x00ff;
    }

    (*encoding)[index].sequence[1] = code.make_code;
    (*encoding)[index].sequence[3] = (code.break_code & 0xff00) >> 8;
    (*encoding)[index].sequence[2] = code.break_code & 0x00ff;

    (*encoding)[index].sequence = malloc((*encoding)[index].sequence_size * sizeof(uint8_t));
}

ps2_scancode get_scan_code(char* str) {
    for(int i = 0; i < scancodes_size; i++) {
        if(strcmp(str, scancodes[i].name) == 0) {
            return scancodes[i];
        }
    }
}

size_t encode_ps2(ps2_encoding *encoding, int encoding_table_size, char *ncurses_str, uint32_t **target) {
    char ascii;
    bool useCtrl = false;
    int size_actual;
    if(strlen(ncurses_str) == 1) {
        ascii = ncurses_str[0];
    }
    if(strlen(ncurses_str) == 2 && ncurses_str[0] == '^') {
        ascii = ncurses_str[1];
        useCtrl = true;
    }
    for(int i = 0; i < encoding_table_size; i++) {
        if(encoding[i].ascii == ascii) {
            size_actual = encoding[i].sequence_size + (useCtrl ? 3 : 0);
            *target = malloc(size_actual);
            memcpy(*target, encoding[i].sequence + (useCtrl ? 1 : 0), size_actual);
            if(useCtrl) {
                ps2_scancode modifier_code = get_scan_code("Ctrl (Left)");
                (*target)[0] = modifier_code.make_code;
                (*target)[encoding[i].sequence_size+2] = (modifier_code.break_code & 0xff00) >> 8;
                (*target)[encoding[i].sequence_size+3] = (modifier_code.break_code & 0x00ff);

            }
            return size_actual;
        }
    }
}
char decode_ps2(ps2_encoding *encoding, uint32_t *source, size_t size) {
  return 0;
}