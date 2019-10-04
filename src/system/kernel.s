;;; kernel.s  Kernel with utilities
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
	JSR [ps2_init]	; init PS/2 driver

	;; test cpu behaivoiur


	;; prepare tape loading
	LDA #$00
	STA $00
	LDA #$02
	STA $01			; store $0200 to $00 for the tape read to know where to put fetched data

	LDA #$04
	STA $31
	
	JSR [tape_read]	; call tape_read subroutine
	JMP $0200

;;; Tape read routines

;;; variables in memory:
;;;   $00, $01 = address
;;;   $02 = input value fetched  in [wait_data]
tape_read:
	LDY #$00
	LDX #$08
wait_data:
	LDA $f7ff			; read from tape interface
	STA $02
	AND #$02	    	; check if read bit is set
	BNE [read_bit]
	LDA $02				; read value previously fetched from tape interface
	AND #$01			; check if data bit is set. if set, data transmition is done
	BEQ [transm_cont]	; else continue
	RTS
transm_cont:	
	BEQ [wait_data]		; keep waiting if zero; else continue

read_bit:
	LDA ($00),Y
	ROL					; rotate current byte
	STA ($00),Y
	LDA $02				; read from value stored from tape interface in wait_data
	AND #$01			; only keep lowest bit
	ADC ($00),Y			; add shifted value to current data position
	STA ($00),Y
	DEX
	BNE [wait_no_data]
prep_next_byte:	
	INY
	LDX #$08

wait_no_data:	
	LDA $f7ff			; read from tape interface
	AND #$02			; check if read bit is set
	BNE [wait_no_data]	; keep waiting if not zero; else continue
	JMP [wait_data]



	.pc $fa00
;;; Print Character Routine
;;;
;;; $0000 and $0001 pointer to character
print_char:
	LDY #$00			; reset Y
	LDX #$08			; setup incrementor
	LDA #$80			; setup mask
	STA $02
pr_next_bit:
	LDA ($00),Y
	AND $02
	BEQ [pr_send_low]
pr_send_high:
	LDA #$01
	STA $f7fe
	JMP [pr_jmp_next]
pr_send_low:
	LDA #$00
	STA $f7fe
pr_jmp_next:
	CLC
	ROR $02
	DEX
	BEQ [pr_return]
	JMP [pr_next_bit]
pr_return:
	RTS


ps2_init:
	LDA #$09
	STA $f7a1
	RTS

;;; PS/2 interface driver   
;;;
;;; The PS/2 device clock triggers an interrupt on low on the clk line and 
;;; either low or high on the data line which is then read by the following 
;;; subroutine:
;;;
;;; (maximum of 77 clockcycles, 111 including interrupt handling. On a 2MHz 
;;; CPU the 10 - 16KHz clock of the PS/2 device gives 125 - 200 clockcycles 
;;; to handle the incoming bit)
ps2_i_read_bit:
	LDA $f7a1			; load bit counter
	SEC
	SBC #$01			; subtract 1
	BEQ [ps2_i_skp_par]	; if bit counter - 1 is zero and therefore the parity bit, skip processing the incoming bit.
	LDX $f7a0			; load byte position into X regs
	CLC
	ROL $f7a2,X			; Rotate left so 
	LDA $f7fd 			; read bit from PS/2 interface
	AND #$01			; only take first bit
	ADC $f7a2,X			; add bit to current byte (and store in accumlator)
	STA $f7a2,X			; store byte
ps2_i_skp_par:			; in any case (parity bit or not)
	DEC $f7a1			; decrement counter 
	BNE [ps2_i_read_end]
	INC $f7a0
	JSR [ps2_init]		; reset bit counter
ps2_i_read_end:
	RTS



;;; Interrupt Handlers
	.pc $ffa0
nmi_handler:
	PHA
	PHP
	LDA $f7fd
	BEQ [nmi_skip]
	JSR [ps2_i_read_bit]
nmi_skip:
	INC $80
	PLP
	PLA
	RTI

	
	.pc $ffc0
irq_handler:
	SEI

	INC $81
	
	RTI