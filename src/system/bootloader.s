

;;; Interrupt and reset vectors:
	.pc $fffa
nmi_vector:	
	.word $ffa0
reset_vector:	
	.word $f800
irq_vector:	
	.word $ffc0
