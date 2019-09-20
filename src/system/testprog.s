	.pc $0200
start:
	JMP [test]
	LDA #$05
	STA $00
loop:	
	JSR [log]
	DEC $00
	BNE [loop]
	JMP [start]

	.pc $0210
log:
	LDA $00
	LDX $02
	STA $04,X
	INX
	STX $02
	RTS

	.pc $0220
test:
	LDA #$00
	STA $0300
	LDX #$00
loop2:
	INC $0300,X
	INX
	JMP [loop2]
	
