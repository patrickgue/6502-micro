	.pc $0200

	LDA #$00
	STA $00
	LDA #$03
	STA $01			; store $0200 to $00 for the tape read to know where to put fetched data

	LDA #$20 ; space
	STA $0300
loop:
	JSR $fa00 ; print_char
	INC $0300
	JMP [loop]