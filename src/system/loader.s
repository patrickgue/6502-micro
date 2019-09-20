

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
	
