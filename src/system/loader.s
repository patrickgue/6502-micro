

	.pc $ff80
entry:
	CLI
	JMP $0200


	.pc $ffa0
nmi_handler:
	PHA
	PHP

	INC $80
	
	PLP
	PLP
	RTI

	
	.pc $ffc0
irq_handler:
	SEI

	INC $81
	
	RTI
	


	.pc $fffa
nmi_vector:	
	.word $ffa0
reset_vector:	
	.word $ff80
irq_vector:	
	.word $ffc0
