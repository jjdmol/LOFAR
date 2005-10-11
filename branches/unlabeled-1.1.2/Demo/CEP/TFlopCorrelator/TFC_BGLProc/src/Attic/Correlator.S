#include <TFC_Interface/TFC_Config.h>

# C[s1][s2][p1][p2][ch] = sum over t of A[ch][s1][p1][t] * ~ A[ch][s2][p2][t]
# Cr = Ar*Br+Ai*Bi, Ci = Ai*Br-Ar*Bi

#	fxcpmadd 1,16,21,1	# f1r += f21r*f16r, f1i += f21i*f16r
#	fxcxnsma 1,16,21,1	# f1r += f21i*f16i, f1i -= f21r*f16i


.global _correlate_2x2
_correlate_2x2:

#	computes correlations of S0*~S2,S0*~S3,S1*~S2,S1*~S3
#	r3 :	pointer to S0 data
#	r4 :	pointer to S1 data
#	r5 :	pointer to S2 data
#	r6 :	pointer to S3 data
#	r7 :	pointer to S0*~S2 output
#	r8 :	pointer to S0*~S3 output
#	r9 :	pointer to S1*~S2 output
#	r10:	pointer to S1*~S3 output

#	local variables:
#	r11:	temporary
#	f0 :	sum of S0_X * ~S2_X
#	f1 :	sum of S0_X * ~S2_Y
#	f2 :	sum of S0_Y * ~S2_X
#	f3 :	sum of S0_Y * ~S2_Y
#	f4 :	sum of S0_X * ~S3_X
#	f5 :	sum of S0_X * ~S3_Y
#	f6 :	sum of S0_Y * ~S3_X
#	f7 :	sum of S0_Y * ~S3_Y
#	f8 :	sum of S1_X * ~S2_X
#	f9 :	sum of S1_X * ~S2_Y
#	f10:	sum of S1_Y * ~S2_X
#	f11:	sum of S1_Y * ~S2_Y
#	f12:	sum of S1_X * ~S3_X
#	f13:	sum of S1_X * ~S3_Y
#	f14:	sum of S1_Y * ~S3_X
#	f15:	sum of S1_Y * ~S3_Y
#	f16,f24:S0_X (sample from station S0, X polarization)
#	f17,f25:S0_Y
#	f18,f26:S1_X
#	f19,f27:S1_Y
#	f20,f28:S2_X
#	f21,f29:S2_Y
#	f22,f30:S3_X
#	f23,f31:S3_Y

	li	 11,NR_SAMPLES_PER_INTEGRATION/2
	mtctr	 11

	li	 11,-16		# push call-saved registers
	stfpdux	 14,1,11
	stfpdux	 15,1,11
	stfpdux	 16,1,11
	stfpdux	 17,1,11
	stfpdux	 18,1,11
	stfpdux	 19,1,11
	stfpdux	 20,1,11
	stfpdux	 21,1,11
	stfpdux	 22,1,11
	stfpdux	 23,1,11
	stfpdux	 24,1,11
	stfpdux	 25,1,11
	stfpdux	 26,1,11
	stfpdux	 27,1,11
	stfpdux	 28,1,11
	stfpdux	 29,1,11
	stfpdux	 30,1,11
	stfpdux	 31,1,11

	li	 11,8		# prefetch station samples
	lfpsx	 16,0,3	
	lfpsux	 17,3,11
	lfpsx	 18,0,4	
	lfpsux	 19,4,11
	lfpsx	 20,0,5	
	lfpsux	 21,5,11
	lfpsx	 22,0,6	
	lfpsux	 23,6,11

	fpsub	 0,0,0		# does this work if f0 is NaN?
	fpsub	 1,1,1
	fpsub	 2,2,2
	fpsub	 3,3,3
	fpsub	 4,4,4
	fpsub	 5,5,5
	fpsub	 6,6,6
	fpsub	 7,7,7
	fpsub	 8,8,8
	fpsub	 9,9,9
	fpsub	 10,10,10
	fpsub	 11,11,11
	fpsub	 12,12,12
	fpsub	 13,13,13
	fpsub	 14,14,14
	fpsub	 15,15,15

loop1:	# loop over time

	lfpsux	 24,3,11	# read station samples
	lfpsux	 25,3,11
	lfpsux	 26,4,11
	lfpsux	 27,4,11
	lfpsux	 28,5,11
	lfpsux	 29,5,11
	lfpsux	 30,6,11
	lfpsux	 31,6,11

	fxcpmadd 0,16,20,0	# S0_X * ~S2_X, phase 1
	fxcpmadd 1,16,21,1	# S0_X * ~S2_Y, phase 1
	fxcpmadd 2,17,20,2	# S0_Y * ~S2_X, phase 1
	fxcpmadd 3,17,21,3	# S0_Y * ~S2_Y, phase 1

	fxcpmadd 4,16,22,4	# S0 * ~S3, phase 1
	fxcpmadd 5,16,23,5
	fxcpmadd 6,17,22,6
	fxcpmadd 7,17,23,7

	fxcpmadd 8,18,20,8	# S1 * ~S2, phase 1
	fxcpmadd 9,18,21,9
	fxcpmadd 10,19,20,10
	fxcpmadd 11,19,21,11

	fxcpmadd 12,18,22,12	# S1 * ~S3, phase 1
	fxcpmadd 13,18,23,13
	fxcpmadd 14,19,22,14
	fxcpmadd 15,19,23,15

	fxcxnsma 0,16,20,0	# S0_X * ~S2_X, phase 2
	fxcxnsma 1,16,21,1	# S0_X * ~S2_Y, phase 2
	fxcxnsma 2,17,20,2	# S0_Y * ~S2_X, phase 2
	fxcxnsma 3,17,21,3	# S0_Y * ~S2_Y, phase 2

	fxcxnsma 4,16,22,4	# S0 * ~S3, phase 2
	fxcxnsma 5,16,23,5
	fxcxnsma 6,17,22,6
	fxcxnsma 7,17,23,7

	fxcxnsma 8,18,20,8	# S1 * ~S2, phase 2
	fxcxnsma 9,18,21,9
	fxcxnsma 10,19,20,10
	fxcxnsma 11,19,21,11

	fxcxnsma 12,18,22,12	# S1 * ~S3, phase 2
	fxcxnsma 13,18,23,13
	fxcxnsma 14,19,22,14
	fxcxnsma 15,19,23,15

	lfpsux	 16,3,11	# f16:f17 = S0
	lfpsux	 17,3,11
	lfpsux	 18,4,11	# f18:f19 = S1
	lfpsux	 19,4,11
	lfpsux	 20,5,11	# f20:f21 = S2
	lfpsux	 21,5,11
	lfpsux	 22,6,11	# f22:f23 = S3
	lfpsux	 23,6,11

	fxcpmadd 0,24,28,0	# S0 * ~S2, phase 1
	fxcpmadd 1,24,29,1	# f1r += f29r*f24r, f1i += f29i*f24r
	fxcpmadd 2,25,28,2
	fxcpmadd 3,25,29,3

	fxcpmadd 4,24,30,4	# S0 * ~S3, phase 1
	fxcpmadd 5,24,31,5
	fxcpmadd 6,25,30,6
	fxcpmadd 7,25,31,7

	fxcpmadd 8,26,28,8	# S1 * ~S2, phase 1
	fxcpmadd 9,26,29,9
	fxcpmadd 10,27,28,10
	fxcpmadd 11,27,29,11

	fxcpmadd 12,26,30,12	# S1 * ~S3, phase 1
	fxcpmadd 13,26,31,13
	fxcpmadd 14,27,30,14
	fxcpmadd 15,27,31,15

	fxcxnsma 0,24,28,0	# S0 * ~S2, phase 2
	fxcxnsma 1,24,29,1	# f1r += f29i*f24i, f1i -= f29r*f24i
	fxcxnsma 2,25,28,2
	fxcxnsma 3,25,29,3

	fxcxnsma 4,24,30,4	# S0 * ~S3, phase 2
	fxcxnsma 5,24,31,5
	fxcxnsma 6,25,30,6
	fxcxnsma 7,25,31,7

	fxcxnsma 8,26,28,8	# S1 * ~S2, phase 2
	fxcxnsma 9,26,29,9
	fxcxnsma 10,27,28,10
	fxcxnsma 11,27,29,11

	fxcxnsma 12,26,30,12	# S1 * ~S3, phase 2
	fxcxnsma 13,26,31,13
	fxcxnsma 14,27,30,14
	fxcxnsma 15,27,31,15

	bdnz	 loop1

	#dcbz	 0,7
	stfpsx	 0,0,7		# store results S0 * ~S2
	stfpsux	 1,7,11
	stfpsux	 2,7,11
	stfpsux	 3,7,11

	#dcbz	 0,8
	stfpsx	 4,0,8		# store results S0 * ~S3
	stfpsux	 5,8,11
	stfpsux	 6,8,11
	stfpsux	 7,8,11

	#dcbz	 0,9
	stfpsx	 8,0,9		# store results S1 * ~S2
	stfpsux	 9,9,11
	stfpsux	 10,9,11
	stfpsux	 11,9,11

	#dcbz	 0,10
	stfpsx	 12,0,10	# store results S1 * ~S3
	stfpsux	 13,10,11
	stfpsux	 14,10,11
	stfpsux	 15,10,11

	li	  11,16		# restore call-saved registers
	lfpdx	  31,0,1
	lfpdux	  30,1,11
	lfpdux	  29,1,11
	lfpdux	  28,1,11
	lfpdux	  27,1,11
	lfpdux	  26,1,11
	lfpdux	  25,1,11
	lfpdux	  24,1,11
	lfpdux	  23,1,11
	lfpdux	  22,1,11
	lfpdux	  21,1,11
	lfpdux	  20,1,11
	lfpdux	  19,1,11
	lfpdux	  18,1,11
	lfpdux	  17,1,11
	lfpdux	  16,1,11
	lfpdux	  15,1,11
	lfpdux	  14,1,11

	addi	  1,1,16	# reset stack pointer

	blr			# return


one:	.long	0x3f800000,0x3f800000


#if NR_STATIONS % 2 == 0
.global _auto_correlate_1_and_2
_auto_correlate_1_and_2:

#	computes correlations of S0*~S0,S0*~S1,S1*~S1
#	r3 :	pointer to S0 data
#	r4 :	pointer to S1 data
#	r5 :	pointer to S0*~S0 output
#	r6 :	pointer to S0*~S1 output
#	r7 :	pointer to S1*~S1 output

#	local variables:
#	r11:	temporary
#	f0 :	sum of S0_X * S0_X
#	f1 :	sum of S0_X * ~S0_Y
#	f3 :	sum of S0_Y * S0_Y
#	f4 :	sum of S0_X * ~S1_X
#	f5 :	sum of S0_X * ~S1_Y
#	f6 :	sum of S0_Y * ~S1_X
#	f7 :	sum of S0_Y * ~S1_Y
#	f8 :	sum of S1_X * S1_X
#	f9 :	sum of S1_X * ~S1_Y
#	f11:	sum of S1_Y * S1_Y
#	f12,f16:S0_X (sample from station S0, X polarization)
#	f13,f17:S0_Y
#	f14,f18:S1_X
#	f15,f19:S1_Y

	li	 11,NR_SAMPLES_PER_INTEGRATION/2
	mtctr	 11

	li	 11,-16		# push call-saved registers
	stfpdux	 14,1,11
	stfpdux	 15,1,11
	stfpdux	 16,1,11
	stfpdux	 17,1,11
	stfpdux	 18,1,11
	stfpdux	 19,1,11

	li	 11,8		# prefetch station samples
	lfpsx	 12,0,3	
	lfpsux	 13,3,11
	lfpsx	 14,0,4	
	lfpsux	 15,4,11

	fpsub	 0,0,0		# does this work if f0 is NaN?
	fpsub	 1,1,1
	fpsub	 2,2,2
	fpsub	 3,3,3
	fpsub	 4,4,4
	fpsub	 5,5,5
	fpsub	 6,6,6
	fpsub	 7,7,7
	fpsub	 8,8,8
	fpsub	 9,9,9
	fpsub	 10,10,10
	fpsub	 11,11,11

loop1:	# loop over time

	lfpsux	 16,3,11	# read station samples
	lfpsux	 17,3,11
	lfpsux	 18,4,11
	lfpsux	 19,4,11

	fpmadd	 0,12,12,0	# S0_X * ~S0_X, phase 1
	fxcpmadd 1,12,13,1	# S0_X * ~S0_Y, phase 1
	fpmadd	 3,13,13,3	# S0_Y * ~S0_Y, phase 1

	fxcpmadd 4,12,14,4	# S0 * ~S1, phase 1
	fxcpmadd 5,12,15,5
	fxcpmadd 6,13,14,6
	fxcpmadd 7,13,15,7

	fpmadd	 8,14,14,8	# S1 * ~S1, phase 1
	fxcpmadd 9,14,15,9
	fpmadd	 11,15,15,11

	fxcxnsma 1,12,13,1	# S0_X * ~S0_Y, phase 2

	fxcxnsma 4,12,14,4	# S0 * ~S1, phase 2
	fxcxnsma 5,12,15,5
	fxcxnsma 6,13,14,6
	fxcxnsma 7,13,15,7

	fxcxnsma 9,14,15,9	# S1 * ~S1, phase 2

	lfpsux	 12,3,11	# f12:f13 = S0
	lfpsux	 13,3,11
	lfpsux	 14,4,11	# f14:f15 = S1
	lfpsux	 15,4,11

	fpmadd	 0,16,16,0	# S0 * ~S0, phase 1
	fxcpmadd 1,16,17,1	# f1r += f17r*f16r, f1i += f17i*f16r
	fpmadd	 3,17,17,3

	fxcpmadd 4,16,18,4	# S0 * ~S1, phase 1
	fxcpmadd 5,16,19,5
	fxcpmadd 6,17,18,6
	fxcpmadd 7,17,19,7

	fpmadd	 8,18,18,8	# S1 * ~S1, phase 1
	fxcpmadd 9,18,19,9
	fpmadd	 11,19,19,11

	fxcxnsma 1,16,17,1	# f1r += f17i*f16i, f1i -= f17r*f16i

	fxcxnsma 4,16,18,4	# S0 * ~S1, phase 2
	fxcxnsma 5,16,19,5
	fxcxnsma 6,17,18,6
	fxcxnsma 7,17,19,7

	fxcxnsma 9,18,19,9	# S1 * ~S1, phase 2

	bdnz	 loop1

	lis	 9,one@ha
	addi	 9,9,one@l
	lfpsx	 12,0,9

	fxcsnsma 0,0,12,0	# f0r += 1*f0i, f0i += -1*f0i = 0
	fxcsnsma 3,3,12,3
	fxcsnsma 8,8,12,8
	fxcsnsma 11,11,12,11

	#dcbz	 0,5
	stfpsx	 0,0,5		# store results S0 * ~S0
	stfpsux	 1,5,11
	fsneg	 1,1
	stfpsux	 1,5,11
	stfpsux	 3,5,11

	#dcbz	 0,6
	stfpsx	 4,0,6		# store results S0 * ~S1
	stfpsux	 5,6,11
	stfpsux	 6,6,11
	stfpsux	 7,6,11

	#dcbz	 0,7
	stfpsx	 8,0,7		# store results S1 * ~S1
	stfpsux	 9,7,11
	fsneg	 9,9
	stfpsux	 9,7,11
	stfpsux	 11,7,11

	li	  11,16		# restore call-saved registers
	lfpdux	  19,1,11
	lfpdux	  18,1,11
	lfpdux	  17,1,11
	lfpdux	  16,1,11
	lfpdux	  15,1,11
	lfpdux	  14,1,11

	addi	  1,1,16	# reset stack pointer

	blr			# return

#else

.global _auto_correlate_1x1
_auto_correlate_1x1:

#	computes auto correlations of S0*~S0
#	r3 :	pointer to S0 data
#	r4 :	pointer to 4 results

	li	 5,NR_SAMPLES_PER_INTEGRATION/2
	mtctr	 5

	li	 8,8

	lfpsx	 8,0,3		# f8:f9 = S0
	lfpsux	 9,3,8

	fpsub	 0,0,0
	fpsub	 1,1,1
	fpsub	 2,2,2
	fpsub	 3,3,3

	lis	 9,one@ha
	addi	 9,9,one@l
	lfpsx	 12,0,9

loop3:	# loop over time

	lfpsux	 10,3,8		# f10:f11 = S0
	lfpsux	 11,3,8

	fpmadd	 0,8,8,0	# S0 * ~S0, phase 1
	fxcpmadd 1,8,9,1	# f1r += f9r*f8r, f1i += f9i*f8r
	fxcxnsma 2,8,9,2	# f2r += f9i*f8i, f2i -= f9r*f8i
	fpmadd	 3,9,9,3

	lfpsux	 8,3,8		# f8:f9 = S0
	lfpsux	 9,3,8

	fpmadd	 0,10,10,0	# S0 * ~S0, phase 1
	fxcpmadd 1,10,11,1	# f1r += f11r*f10r, f1i += f11i*f10r
	fxcxnsma 2,10,11,2	# f2r += f11i*f10i, f2i -= f11r*f10i
	fpmadd	 3,11,11,3

	bdnz	 loop3

	fxcsnsma 0,0,12,0	# f0r += 1*f0i, f0i += -1*f0i = 0
	fxcsnsma 3,3,12,3
	fpadd	 1,1,2

	#dcbz	 0,4
	stfpsux	 0,0,4
	stfpsux	 1,4,8
	fsneg	 1,1
	stfpsux	 1,4,8
	stfpsux	 3,4,8

	blr			# return

#endif
