;;; testprog.s  Program to test assembler and emulator functionality
;;; Copyright (C) 2019 Patrick Günthard
;;; 
;;; This program is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation, either version 3 of the License, or
;;;   (at your option) any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program.  If not, see <https://www.gnu.org/licenses/>.


print_char = $fa00
char_store = $0300
	.pc $0200

	LDA #$00
	STA $00
	LDA #$03
	STA $01				; store $0200 to $00 for the tape read to know where to put fetched data
	
	LDX #$00
	LDA #$20 ; space
	STA $0300
loop:
	JSR [print_char]	; print_char
	INC [char_store]
	LDA [char_store],X
	JMP [loop]