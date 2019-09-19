	.pc $0200
start:	
	LDA #$05
	STA $00
loop:	
	DEC $00
	BNE [loop]
	JMP [forward]


	.pc $0300
forward:
	LDA #$20
	STA $01
