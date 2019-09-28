/*
  ps2.h  defines PS/2 scancodes
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
#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#include <stdio.h>

enum s_ps2_modifier {
    NONE,
    SHIFT,
    CTRL
};

typedef enum s_ps2_modifier ps2_modifier;

struct s_ps2_scancode {
    char name[15];
    uint8_t make_code;
    uint16_t break_code;
};

typedef struct s_ps2_scancode ps2_scancode;

struct s_ps2_encoding {
    char ascii;
    uint8_t *sequence;
    int sequence_size;
};

typedef struct s_ps2_encoding ps2_encoding;

static int scancodes_size = 101;
static ps2_scancode scancodes[101] = {
    {" ", 0x52, 0xF052},
    {"1", 0x16, 0xF016},
    {"2", 0x1E, 0xF01E},
    {"3", 0x26, 0xF026},
    {"4", 0x25, 0xF025},
    {"5", 0x2E, 0xF02E},
    {"6", 0x36, 0xF036},
    {"7", 0x3D, 0xF03D},
    {"8", 0x3E, 0xF03E},
    {"9", 0x46, 0xF046},
    {"0", 0x45, 0xF045},
    {"A", 0x1C, 0xF01C},
    {"B", 0x32, 0xF032},
    {"C", 0x21, 0xF021},
    {"D", 0x23, 0xF023},
    {"E", 0x24, 0xF024},
    {"F", 0x2B, 0xF02B},
    {"G", 0x34, 0xF034},
    {"H", 0x33, 0xF033},
    {"I", 0x43, 0xF043},
    {"J", 0x3B, 0xF03B},
    {"K", 0x42, 0xF042},
    {"L", 0x4B, 0xF04B},
    {"M", 0x3A, 0xF03A},
    {"N", 0x31, 0xF031},
    {"O", 0x44, 0xF044},
    {"P", 0x4D, 0xF04D},
    {"Q", 0x15, 0xF015},
    {"R", 0x2D, 0xF02D},
    {"S", 0x1B, 0xF01B},
    {"T", 0x2C, 0xF02C},
    {"U", 0x3C, 0xF03C},
    {"V", 0x2A, 0xF02A},
    {"W", 0x1D, 0xF01D},
    {"X", 0x22, 0xF022},
    {"Y", 0x35, 0xF035},
    {"Z", 0x1A, 0xF01A},
    {"`", 0x0E, 0xF00E},
    {"-", 0x4E, 0xF04E},
    {"=", 0x55, 0xF055},
    {"Backspace", 0x66, 0xF066},
    {"Tab", 0x0D, 0xF00D},
    {"[", 0x54, 0xF054},
    {"]", 0x5B, 0xF05B},
    {"\\", 0x5D, 0xF05D},
    {";", 0x4C, 0xF04C},
    {"Enter", 0x5A, 0xF05A},
    {"Shift (Left)", 0x12, 0xF012},
    {",", 0x41, 0xF041},
    {".", 0x49, 0xF049},
    {"/", 0x4A, 0xF04A},
    {"Caps Lock", 0x58, 0xF058},
    {"ESC", 0x76, 0xF076},
    {"F1", 0x05, 0xF005},
    {"F2", 0x06, 0xF006},
    {"F3", 0x04, 0xF004},
    {"F4", 0x0C, 0xF00C},
    {"F5", 0x03, 0xF003},
    {"F6", 0x0B, 0xF00B},
    {"F7", 0x83, 0xF083},
    {"F8", 0x0A, 0xF00A},
    {"F9", 0x01, 0xF001},
    {"F10", 0x09, 0xF009},
    {"F11", 0x78, 0xF078},
    {"F12", 0x07, 0xF007},
    {"Shift (Right)", 0x59, 0xF059},
    {"Ctrl (Left)", 0x14, 0xF014},
    {"Alt (Left)", 0x11, 0xF011},
    {"Spacebar", 0x29, 0xF029},
    /*{"Alt (right)", 0xE011, 0xE0F011},
    {"Windows (right)", 0xE027, 0xE0F027},
    {"Menus", 0xE02F, 0xE0F02F},
    {"Ctrl (right)", 0xE014, 0xE0F014},
    {"Insert", 0xE070, 0xE0F070},
    {"Home", 0xE06C, 0xE0F06C},
    {"Page Up", 0xE07D, 0xE0F07D},
    {"Delete", 0xE071, 0xE0F071},
    {"End", 0xE069, 0xE0F069},
    {"Page Down", 0xE07A, 0xE0F07A},
    {"Up Arrow", 0xE075, 0xE0F075},
    {"Left Arrow", 0xE06B, 0xE0F06B},
    {"Down Arrow", 0xE072, 0xE0F072},
    {"Right Arrow", 0xE074, 0xE0F074},
    {"Num Lock", 0x77, 0xF077},
    // numpad:
    {"/", 0xE04A, 0xE0F04A},
    {"*", 0x7C, 0xF07C},
    {"-", 0x7B, 0xF07B},
    {"7", 0x6C, 0xF06C},
    {"8", 0x75, 0xF075},
    {"9", 0x7D, 0xF07D},
    {"+", 0x79, 0xF079},
    {"4", 0x6B, 0xF06B},
    {"5", 0x73, 0xF073},
    {"6", 0x74, 0xF074},
    {"1", 0x69, 0xF069},
    {"2", 0x72, 0xF072},
    {"3", 0x7A, 0xF07A},
    {"0", 0x70, 0xF070},
    {".", 0x71, 0xF071},
    {"Enter", 0xE05A, 0xE0F05A}*/
};

int init_ps2_encodings(ps2_encoding **);
void add_ps2_encoding(ps2_encoding **, int, char, ps2_scancode, ps2_modifier);
ps2_scancode get_scan_code(char*);

size_t encode_ps2(ps2_encoding*, int, char *, uint8_t**);
char decode_ps2(ps2_encoding*,uint32_t *, size_t);

#endif