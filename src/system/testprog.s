	.pc $0200
start:	
	LDA #$05
	STA $00
loop:	
	JSR [store]
	DEC $00
	BNE [loop]
	BRK

	.pc $0210
store:
	LDA $00
	LDX $02
	STA $04,X
	INX
	STX $02
	RTS
