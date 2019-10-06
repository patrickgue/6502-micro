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