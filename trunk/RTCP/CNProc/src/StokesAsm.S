#if defined HAVE_BGP

constants:	.long	0x3F800000, 0xBF800000
		.long	0xBF800000, 0x3F800000

.global	_StokesIQUV
_StokesIQUV:
	srwi	8,8,2		# set loop iterations
	subi	8,8,1
	mtctr	8

	li	12,-16		# push call-saved registers
				; stfpdux	14,1,12
				; stfpdux	15,1,12
				; stfpdux	16,1,12
				; stfpdux	17,1,12
				; stfpdux	18,1,12
				; stfpdux	19,1,12
				; stfpdux	20,1,12
				; stfpdux	21,1,12
				; stfpdux	22,1,12
				; stfpdux	23,1,12
				; stfpdux	24,1,12

	li	8,8

	lis	9,constants@ha	# load constants
	addi	9,9,constants@l
	lfpsx	24,0,9
	lfpsux	25,9,8

	li	9,4

	subi	3,3,4
	subi	4,4,4
	subi	5,5,4
	subi	6,6,4
	subi	7,7,8

				; lfpsux	18,7,8
				; lfpsux	19,7,8
				; lfpsux	0,7,8
				; lfpsux	1,7,8
				; lfpsux	6,7,8
	fpmul	20,18,18
				; lfpsux	7,7,8
	fpmul	21,19,19
	fxpmul	23,19,18
				; lfpsux	12,7,8
	fpmul	2,0,0
	fxcxnsma 23,19,18,23
				; lfpsux	13,7,8
	fpmul	3,1,1
	fxpmul	5,1,0
	fxmadd	22,21,24,20
	fxmadd	21,21,25,20
	fpadd	23,23,23
				; lfpsux	18,7,8
	fpmul	8,6,6
	fxcxnsma 5,1,0,5
				; lfpsux	19,7,8
	fpmul	9,7,7
	fxpmul	11,7,6

0:
	fxcxma	22,25,21,22
	fxmadd	4,3,24,2
				; stfsux	23,5,9
	fxmadd	3,3,25,2
				; stfssux	23,6,9
	fpadd	5,5,5
				; lfpsux	0,7,8
	fpmul	14,12,12
	fxcxnsma 11,7,6,11
				; lfpsux	1,7,8
	fpmul	15,13,13
				; stfsux	22,3,9
	fxpmul	17,13,12
				; stfssux	22,4,9

	fxcxma	4,25,3,4
	fxmadd	10,9,24,8
				; stfsux	5,5,9
	fxmadd	9,9,25,8
				; stfssux	5,6,9
	fpadd	11,11,11
				; lfpsux	6,7,8
	fpmul	20,18,18
	fxcxnsma 17,13,12,17
				; lfpsux	7,7,8
	fpmul	21,19,19
				; stfsux	4,3,9
	fxpmul	23,19,18
				; stfssux	4,4,9

	fxcxma	10,25,9,10
	fxmadd	16,15,24,14
				; stfsux	11,5,9
	fxmadd	15,15,25,14
				; stfssux	11,6,9
	fpadd	17,17,17
				; lfpsux	12,7,8
	fpmul	2,0,0
	fxcxnsma 23,19,18,23
				; lfpsux	13,7,8
	fpmul	3,1,1
				; stfsux	10,3,9
	fxpmul	5,1,0
				; stfssux	10,4,9

	fxcxma	16,25,15,16
	fxmadd	22,21,24,20
				; stfsux	17,5,9
	fxmadd	21,21,25,20
				; stfssux	17,6,9
	fpadd	23,23,23
				; lfpsux	18,7,8
	fpmul	8,6,6
	fxcxnsma 5,1,0,5
				; lfpsux	19,7,8
	fpmul	9,7,7
				; stfsux	16,3,9
	fxpmul	11,7,6
				; stfssux	16,4,9

	bdnz	0b

	fxcxma	22,25,21,22
	fxmadd	4,3,24,2
				; stfsux	23,5,9
	fxmadd	3,3,25,2
				; stfssux	23,6,9
	fpadd	5,5,5
	fpmul	14,12,12
	fxcxnsma 11,7,6,11
	fpmul	15,13,13
				; stfsux	22,3,9
	fxpmul	17,13,12
				; stfssux	22,4,9

	fxcxma	4,25,3,4
	fxmadd	10,9,24,8
				; stfsux	5,5,9
	fxmadd	9,9,25,8
				; stfssux	5,6,9
	fpadd	11,11,11
	fxcxnsma 17,13,12,17
				; stfsux	4,3,9
				; stfssux	4,4,9

	fxcxma	10,25,9,10
	fxmadd	16,15,24,14
				; stfsux	11,5,9
	fxmadd	15,15,25,14
				; stfssux	11,6,9
	fpadd	17,17,17
				; stfsux	10,3,9
				; stfssux	10,4,9

	fxcxma	16,25,15,16
				; stfsux	17,5,9
				; stfssux	17,6,9
				; stfsux	16,3,9
				; stfssux	16,4,9

	li	12,16		# restore call-saved registers
				; lfpdx		24,0,1
				; lfpdux	23,1,12	
				; lfpdux	22,1,12	
				; lfpdux	21,1,12	
				; lfpdux	20,1,12	
				; lfpdux	19,1,12	
				; lfpdux	18,1,12	
				; lfpdux	17,1,12	
				; lfpdux	16,1,12	
				; lfpdux	15,1,12	
				; lfpdux	14,1,12	

	addi	1,1,16		# reset stack pointer
	blr

#endif
