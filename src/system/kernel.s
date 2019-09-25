;;; loader.s  Bootloader and basic Kernel
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


	.pc $f800
kernel_main:
	CLI			; enable irq interrupts
	
	;; prepare tape loading
	;LDA #$00
	;STA $00
	;LDA #$02
	;STA $01			; store $0200 to $00 for the tape read to know where to put fetched data
	;JSR [tape_read]		; call tape_read subroutine

	;; execute loaded program
	JMP $0200 		; jump to main


;;; Tape read routines

;;; variables in memory:
;;;   $00, $01 = address
;;;   $02 = temp value
;;;   $03 = byte position decrementor
;;;   $04 = multi purpose counter
tape_read:
	LDA #$00
	TAY
	LDA #$08
	STA $02
wait_data:
	LDA $f7ff		; read from tape interface
	STA $05
	AND #$02	    	; check if read bit is set
	BNE [read_bit]
	LDA $05			; read value previously fetched from tape interface
	AND #$01		; check if data bit is set. if set, data transmition is done
	BEQ [transm_cont]	; else continue
	RTS
transm_cont:	
	BEQ [wait_data]		; keep waiting if zero; else continue

read_bit:
	LDA ($00),Y
	ROL			; rotate current byte
	STA ($00),Y
	LDA $05			; read from value stored from tape interface in wait_data
	AND #$01		; only keep lowest bit
	ADC ($00),Y		; add shifted value to current data position
	DEC $02
	BNE [wait_no_data]
prep_next_byte:	
	INY
	LDA #$08
	STA $02
	
wait_no_data:	
	LDA $f7ff		; read from tape interface
	AND #$02		; check if read bit is set
	BNE [wait_no_data]	; keep waiting if not zero; else continue
	JMP [wait_data]




;;; Interrupt Handlers
	.pc $ffa0
nmi_handler:
	
	INC $80
	
	RTI

	
	.pc $ffc0
irq_handler:
	SEI

	INC $81
	
	RTI