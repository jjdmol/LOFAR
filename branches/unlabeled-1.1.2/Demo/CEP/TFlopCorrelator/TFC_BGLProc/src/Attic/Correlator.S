# C[s1][s2][p1][p2][ch] = sum over t of A[ch][s1][p1][t] * ~ A[ch][s2][p2][t]
# Cr = Ar*Br+Ai*Bi, Ci = Ai*Br-Ar*Bi



.global _correlate_2x2
_correlate_2x2:

#	computes correlations of S0*S2,S1*S2,S0*S3,S1*S3
#	r3 :	pointer to S0 data
#	r4 :	pointer to S1 data
#	r5 :	pointer to S2 data
#	r6 :	pointer to S3 data
#	r7 :	pointer to 4*4 results
#	r8 :	#time samples

	mtctr	8
	li	9,-16

	stfpdux	14,1,9	# push call-saved registers
	stfpdux	15,1,9	
	stfpdux	16,1,9
	stfpdux	17,1,9
	stfpdux	18,1,9
	stfpdux	19,1,9
	stfpdux	20,1,9
	stfpdux	21,1,9
	stfpdux	22,1,9
	stfpdux	23,1,9
	stfpdux	24,1,9
	stfpdux	25,1,9
	stfpdux	26,1,9
	stfpdux	27,1,9
	stfpdux	28,1,9
	stfpdux	29,1,9
	stfpdux	30,1,9
	stfpdux	31,1,9

	li	8,8

	sub	3,3,8
	sub	4,4,8
	sub	5,5,8
	sub	6,6,8
	sub	7,7,8

	fpsub	0,0,0	# does this work if f0 is NaN?
	fpsub	1,1,1
	fpsub	2,2,2
	fpsub	3,3,3
	fpsub	4,4,4
	fpsub	5,5,5
	fpsub	6,6,6
	fpsub	7,7,7
	fpsub	8,8,8
	fpsub	9,9,9
	fpsub	10,10,10
	fpsub	11,11,11
	fpsub	12,12,12
	fpsub	13,13,13
	fpsub	14,14,14
	fpsub	15,15,15

loop1:	# loop over time

	lfpsux	24,3,8		# f24:f25 = S0
	lfpsux	25,3,8
	lfpsux	26,4,8		# f26:f27 = S1
	lfpsux	27,4,8
	lfpsux	28,5,8		# f28:f29 = S2
	lfpsux	29,5,8
	lfpsux	30,6,8		# f30:f31 = S3
	lfpsux	31,6,8

	fxcpmadd 0,24,28,0	# S0 * ~S2, phase 1
	fxcpmadd 1,24,29,1	# f0r+=f29r*f24r, f0i += f29i*f24r
	fxcpmadd 2,25,28,2
	fxcpmadd 3,25,29,3

	fxcpmadd 4,26,28,4	# S1 * ~S2, phase 1
	fxcpmadd 5,26,29,5
	fxcpmadd 6,27,28,6
	fxcpmadd 7,27,29,7

	fxcxnsma 0,24,28,0	# S0 * ~S2, phase 2
	fxcxnsma 1,24,29,1	# f0r+=f29i*f24i, f0i -= f29r*f24i
	fxcxnsma 2,25,28,2
	fxcxnsma 3,25,29,3

	fxcxnsma 4,26,28,4	# S1 * ~S2, phase 2
	fxcxnsma 5,26,29,5
	fxcxnsma 6,27,28,6
	fxcxnsma 7,27,29,7

	fxcpmadd 8,24,30,8	# S0 * ~S3, phase 1
	fxcpmadd 9,24,31,9
	fxcpmadd 10,25,30,10
	fxcpmadd 11,25,31,11

	fxcpmadd 12,26,30,12	# S1 * ~S3, phase 1
	fxcpmadd 13,26,31,13
	fxcpmadd 14,27,30,14
	fxcpmadd 15,27,31,15

	fxcxnsma 8,24,30,8	# S0 * ~S3, phase 2
	fxcxnsma 9,24,31,9
	fxcxnsma 10,25,30,10
	fxcxnsma 11,25,31,11

	fxcxnsma 12,26,30,12	# S1 * ~S3, phase 2
	fxcxnsma 13,26,31,13
	fxcxnsma 14,27,30,14
	fxcxnsma 15,27,31,15

	bdnz	loop1

	stfpsux	 0,7,8		# store results
	stfpsux	 1,7,8
	stfpsux	 2,7,8
	stfpsux	 3,7,8
	stfpsux	 4,7,8
	stfpsux	 5,7,8
	stfpsux	 6,7,8
	stfpsux	 7,7,8
	stfpsux	 8,7,8
	stfpsux	 9,7,8
	stfpsux	 10,7,8
	stfpsux	 11,7,8
	stfpsux	 12,7,8
	stfpsux	 13,7,8
	stfpsux	 14,7,8
	stfpsux	 15,7,8

	la	8,-16(1)	# restore call-saved registers
	li	9,16
	lfpdux	14,8,9
	lfpdux	15,8,9
	lfpdux	16,8,9
	lfpdux	17,8,9
	lfpdux	18,8,9
	lfpdux	19,8,9
	lfpdux	20,8,9
	lfpdux	21,8,9
	lfpdux	22,8,9
	lfpdux	23,8,9
	lfpdux	24,8,9
	lfpdux	25,8,9
	lfpdux	26,8,9
	lfpdux	27,8,9
	lfpdux	28,8,9
	lfpdux	29,8,9
	lfpdux	30,8,9
	lfpdux	31,8,9

	addi	1,1,288		# reset stack pointer

	blr			# return



.global _correlate_2x3
_correlate_2x3:

#	computes correlations of S0*S2,S1*S2,S0*S3,S1*S3,S0*S4,S1*S4
#	r3 :	pointer to S0 data
#	r4 :	pointer to S1 data
#	r5 :	pointer to S2 data
#	r6 :	pointer to S3 data
#	r7 :	pointer to S4 data
#	r8 :	pointer to 6*4 results
#	r9 :	#time samples

	mtctr	9
	li	9,-16

	stfpdux	14,1,9	# push call-saved registers
	stfpdux	15,1,9	
	stfpdux	16,1,9
	stfpdux	17,1,9
	stfpdux	18,1,9
	stfpdux	19,1,9
	stfpdux	20,1,9
	stfpdux	21,1,9
	stfpdux	22,1,9
	stfpdux	23,1,9
	stfpdux	24,1,9
	stfpdux	25,1,9
	stfpdux	26,1,9
	stfpdux	27,1,9
	stfpdux	28,1,9
	stfpdux	29,1,9
	stfpdux	30,1,9
	stfpdux	31,1,9

	li	9,8

	sub	3,3,9
	sub	4,4,9
	sub	5,5,9
	sub	6,6,9
	sub	7,7,9
	sub	8,8,9

	fpsub	0,0,0	# does this work if f0 is NaN?
	fpsub	1,1,1
	fpsub	2,2,2
	fpsub	3,3,3
	fpsub	4,4,4
	fpsub	5,5,5
	fpsub	6,6,6
	fpsub	7,7,7
	fpsub	8,8,8
	fpsub	9,9,9
	fpsub	10,10,10
	fpsub	11,11,11
	fpsub	12,12,12
	fpsub	13,13,13
	fpsub	14,14,14
	fpsub	15,15,15
	fpsub	16,16,16
	fpsub	17,17,17
	fpsub	18,18,18
	fpsub	19,19,19
	fpsub	20,20,20
	fpsub	21,21,21
	fpsub	22,22,22
	fpsub	23,23,23

loop2:	# loop over time

	lfpsux	24,3,9		# f24:f25 = S0
	lfpsux	25,3,9
	lfpsux	26,4,9		# f26:f27 = S1
	lfpsux	27,4,9
	lfpsux	28,5,9		# f28:f29 = S2
	lfpsux	29,5,9
	lfpsux	30,6,9		# f30:f31 = S3
	lfpsux	31,6,9

	fxcpmadd 0,24,28,0	# S0 * ~S2, phase 1
	fxcpmadd 1,24,29,1	# f0r+=f29r*f24r, f0i += f29i*f24r
	fxcpmadd 2,25,28,2
	fxcpmadd 3,25,29,3

	fxcpmadd 4,26,28,4	# S1 * ~S2, phase 1
	fxcpmadd 5,26,29,5
	fxcpmadd 6,27,28,6
	fxcpmadd 7,27,29,7

	fxcxnsma 0,24,28,0	# S0 * ~S2, phase 2
	fxcxnsma 1,24,29,1	# f0r+=f29i*f24i, f0i -= f29r*f24i
	fxcxnsma 2,25,28,2
	fxcxnsma 3,25,29,3

	fxcxnsma 4,26,28,4	# S1 * ~S2, phase 2
	fxcxnsma 5,26,29,5
	fxcxnsma 6,27,28,6
	fxcxnsma 7,27,29,7

	lfpsux	28,7,9		# f28:f29 = S4
	lfpsux  29,7,9

	fxcpmadd 8,24,30,8	# S0 * ~S3, phase 1
	fxcpmadd 9,24,31,9
	fxcpmadd 10,25,30,10
	fxcpmadd 11,25,31,11

	fxcpmadd 12,26,30,12	# S1 * ~S3, phase 1
	fxcpmadd 13,26,31,13
	fxcpmadd 14,27,30,14
	fxcpmadd 15,27,31,15

	fxcxnsma 8,24,30,8	# S0 * ~S3, phase 2
	fxcxnsma 9,24,31,9
	fxcxnsma 10,25,30,10
	fxcxnsma 11,25,31,11

	fxcxnsma 12,26,30,12	# S1 * ~S3, phase 2
	fxcxnsma 13,26,31,13
	fxcxnsma 14,27,30,14
	fxcxnsma 15,27,31,15

	fxcpmadd 16,24,28,16	# S0 * ~S4, phase 1
	fxcpmadd 17,24,29,17
	fxcpmadd 18,25,28,18
	fxcpmadd 19,25,29,19

	fxcpmadd 20,26,28,20	# S1 * ~S4, phase 1
	fxcpmadd 21,26,29,21
	fxcpmadd 22,27,28,22
	fxcpmadd 23,27,29,23

	fxcxnsma 16,24,28,16	# S0 * ~S4, phase 2
	fxcxnsma 17,24,29,17
	fxcxnsma 18,25,28,18
	fxcxnsma 19,25,29,19

	fxcxnsma 20,26,28,20	# S1 * ~S4, phase 2
	fxcxnsma 21,26,29,21
	fxcxnsma 22,27,28,22
	fxcxnsma 23,27,29,23

	bdnz	loop2

	stfpsux	 0,8,9		# store results
	stfpsux	 1,8,9
	stfpsux	 2,8,9
	stfpsux	 3,8,9
	stfpsux	 4,8,9
	stfpsux	 5,8,9
	stfpsux	 6,8,9
	stfpsux	 7,8,9
	stfpsux	 8,8,9
	stfpsux	 9,8,9
	stfpsux	 10,8,9
	stfpsux	 11,8,9
	stfpsux	 12,8,9
	stfpsux	 13,8,9
	stfpsux	 14,8,9
	stfpsux	 15,8,9
	stfpsux	 16,8,9
	stfpsux	 17,8,9
	stfpsux	 18,8,9
	stfpsux	 19,8,9
	stfpsux	 20,8,9
	stfpsux	 21,8,9
	stfpsux	 22,8,9
	stfpsux	 23,8,9

	la	8,-16(1)	# restore call-saved registers
	li	9,16
	lfpdux	14,8,9
	lfpdux	15,8,9
	lfpdux	16,8,9
	lfpdux	17,8,9
	lfpdux	18,8,9
	lfpdux	19,8,9
	lfpdux	20,8,9
	lfpdux	21,8,9
	lfpdux	22,8,9
	lfpdux	23,8,9
	lfpdux	24,8,9
	lfpdux	25,8,9
	lfpdux	26,8,9
	lfpdux	27,8,9
	lfpdux	28,8,9
	lfpdux	29,8,9
	lfpdux	30,8,9
	lfpdux	31,8,9

	addi	1,1,288		# reset stack pointer

	blr			# return
