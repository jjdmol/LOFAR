#if defined HAVE_BGL
#include <TFC_Interface/TFC_Config.h>

# C[s1][s2][p1][p2][ch] = sum over t of A[ch][s1][p1][t] * ~ A[ch][s2][p2][t]
# Cr = Ar*Br+Ai*Bi, Ci = Ai*Br-Ar*Bi

#	fxcpnsma 1,16,21,1	# f1r += f21r*f16r, f1i -= f21i*f16r
#	fxcxma   1,16,21,1	# f1r += f21i*f16i, f1i += f21r*f16i

zero:	.long	0,0
one:	.long	0x3f800000,0x3f800000



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
#	f4 :	sum of S1_X * ~S2_X
#	f5 :	sum of S1_X * ~S2_Y
#	f6 :	sum of S1_Y * ~S2_X
#	f7 :	sum of S1_Y * ~S2_Y
#	f8 :	sum of S0_X * ~S3_X
#	f9 :	sum of S0_X * ~S3_Y
#	f10:	sum of S0_Y * ~S3_X
#	f11:	sum of S0_Y * ~S3_Y
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

	lis	 12,zero@ha
	addi	 12,12,zero@l
	lfpsx	 0,0,12

	li	 11,8		# prefetch station samples
	lfpsx	 16,0,3	
	lfpsux	 17,3,11
	lfpsx	 18,0,4	
	lfpsux	 19,4,11
	lfpsx	 20,0,5	
	lfpsux	 21,5,11
	lfpsx	 22,0,6	
	lfpsux	 23,6,11

	fpmr	 1,0
	fpmr	 2,0
	fpmr	 3,0
	fpmr	 4,0
	fpmr	 5,0
	fpmr	 6,0
	fpmr	 7,0
	fpmr	 8,0
	fpmr	 9,0
	fpmr	 10,0
	fpmr	 11,0
	fpmr	 12,0
	fpmr	 13,0
	fpmr	 14,0
	fpmr	 15,0

loop1:	# loop over time

	lfpsux	 24,3,11	# read station samples
	lfpsux	 25,3,11
	lfpsux	 26,4,11
	lfpsux	 27,4,11
	lfpsux	 28,5,11
	lfpsux	 29,5,11
	lfpsux	 30,6,11
	lfpsux	 31,6,11

	fxcpnsma 0,16,20,0	# S0_X * ~S2_X, phase 1
	fxcpnsma 1,16,21,1	# S0_X * ~S2_Y, phase 1
	fxcpnsma 2,17,20,2	# S0_Y * ~S2_X, phase 1
	fxcpnsma 3,17,21,3	# S0_Y * ~S2_Y, phase 1

	fxcpnsma 4,18,20,4	# S1 * ~S2, phase 1
	fxcpnsma 5,18,21,5
	fxcpnsma 6,19,20,6
	fxcpnsma 7,19,21,7

	fxcpnsma 8,16,22,8	# S0 * ~S3, phase 1
	fxcpnsma 9,16,23,9
	fxcpnsma 10,17,22,10
	fxcpnsma 11,17,23,11

	fxcpnsma 12,18,22,12	# S1 * ~S3, phase 1
	fxcpnsma 13,18,23,13
	fxcpnsma 14,19,22,14
	fxcpnsma 15,19,23,15

	fxcxma   0,16,20,0	# S0_X * ~S2_X, phase 2
	fxcxma   1,16,21,1	# S0_X * ~S2_Y, phase 2
	fxcxma   2,17,20,2	# S0_Y * ~S2_X, phase 2
	fxcxma   3,17,21,3	# S0_Y * ~S2_Y, phase 2

	fxcxma   4,18,20,4	# S1 * ~S2, phase 2
	fxcxma   5,18,21,5
	fxcxma   6,19,20,6
	fxcxma   7,19,21,7

	fxcxma   8,16,22,8	# S0 * ~S3, phase 2
	fxcxma   9,16,23,9
	fxcxma   10,17,22,10
	fxcxma   11,17,23,11

	fxcxma   12,18,22,12	# S1 * ~S3, phase 2
	fxcxma   13,18,23,13
	fxcxma   14,19,22,14
	fxcxma   15,19,23,15

	lfpsux	 16,3,11	# f16:f17 = S0
	lfpsux	 17,3,11
	lfpsux	 18,4,11	# f18:f19 = S1
	lfpsux	 19,4,11
	lfpsux	 20,5,11	# f20:f21 = S2
	lfpsux	 21,5,11
	lfpsux	 22,6,11	# f22:f23 = S3
	lfpsux	 23,6,11

	fxcpnsma 0,24,28,0	# S0 * ~S2, phase 1
	fxcpnsma 1,24,29,1	# f1r += f29r*f24r, f1i -= f29i*f24r
	fxcpnsma 2,25,28,2
	fxcpnsma 3,25,29,3

	fxcpnsma 4,26,28,4	# S1 * ~S2, phase 1
	fxcpnsma 5,26,29,5
	fxcpnsma 6,27,28,6
	fxcpnsma 7,27,29,7

	fxcpnsma 8,24,30,8	# S0 * ~S3, phase 1
	fxcpnsma 9,24,31,9
	fxcpnsma 10,25,30,10
	fxcpnsma 11,25,31,11

	fxcpnsma 12,26,30,12	# S1 * ~S3, phase 1
	fxcpnsma 13,26,31,13
	fxcpnsma 14,27,30,14
	fxcpnsma 15,27,31,15

	fxcxma   0,24,28,0	# S0 * ~S2, phase 2
	fxcxma   1,24,29,1	# f1r += f29i*f24i, f1i += f29r*f24i
	fxcxma   2,25,28,2
	fxcxma   3,25,29,3

	fxcxma   4,26,28,4	# S1 * ~S2, phase 2
	fxcxma   5,26,29,5
	fxcxma   6,27,28,6
	fxcxma   7,27,29,7

	fxcxma   8,24,30,8	# S0 * ~S3, phase 2
	fxcxma   9,24,31,9
	fxcxma   10,25,30,10
	fxcxma   11,25,31,11

	fxcxma   12,26,30,12	# S1 * ~S3, phase 2
	fxcxma   13,26,31,13
	fxcxma   14,27,30,14
	fxcxma   15,27,31,15

	bdnz	 loop1

	dcbz	 0,7
	stfpsx	 0,0,7		# store results S0 * ~S2
	stfpsux	 1,7,11
	stfpsux	 2,7,11
	stfpsux	 3,7,11

	dcbz	 0,8
	stfpsx	 4,0,8		# store results S1 * ~S2
	stfpsux	 5,8,11
	stfpsux	 6,8,11
	stfpsux	 7,8,11

	dcbz	 0,9
	stfpsx	 8,0,9		# store results S0 * ~S3
	stfpsux	 9,9,11
	stfpsux	 10,9,11
	stfpsux	 11,9,11

	dcbz	 0,10
	stfpsx	 12,0,10	# store results S1 * ~S3
	stfpsux	 13,10,11
	stfpsux	 14,10,11
	stfpsux	 15,10,11

	li	 11,16		# restore call-saved registers
	lfpdx	 31,0,1
	lfpdux	 30,1,11
	lfpdux	 29,1,11
	lfpdux	 28,1,11
	lfpdux	 27,1,11
	lfpdux	 26,1,11
	lfpdux	 25,1,11
	lfpdux	 24,1,11
	lfpdux	 23,1,11
	lfpdux	 22,1,11
	lfpdux	 21,1,11
	lfpdux	 20,1,11
	lfpdux	 19,1,11
	lfpdux	 18,1,11
	lfpdux	 17,1,11
	lfpdux	 16,1,11
	lfpdux	 15,1,11
	lfpdux	 14,1,11

	addi	 1,1,16		# reset stack pointer

	blr			# return


.global _correlate_3x2
_correlate_3x2:

#	computes correlations of S0*~S2,S0*~S3,S0*~S4,S1*~S2,S1*~S3,S1*~S4

	lis	 11,zero@ha
	addi	 11,11,zero@l
	lfpsx	 0,0,11

	li	 0,NR_SAMPLES_PER_INTEGRATION/2
	mtctr	 0

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

	li	 11,8

	lfpsux	 24,0,3		# ld S0
	lfpsux	 25,3,11
	lfpsux	 26,0,4		# ld S1
	lfpsux	 27,4,11
	lfpsux	 28,0,5		# ld S2
	lfpsux	 29,5,11
	lfpsux	 30,0,6		# ld S3
	lfpsux	 31,6,11
	sub	 7,7,11

	fpmr	 1,0
	fpmr	 2,0
	fpmr	 3,0
	fpmr	 4,0
	fpmr	 5,0
	fpmr	 6,0
	fpmr	 7,0
	fpmr	 8,0
	fpmr	 9,0
	fpmr	 10,0
	fpmr	 11,0
	fpmr	 12,0
	fpmr	 13,0
	fpmr	 14,0
	fpmr	 15,0
	fpmr	 16,0
	fpmr	 17,0
	fpmr	 18,0
	fpmr	 19,0
	fpmr	 20,0
	fpmr	 21,0
	fpmr	 22,0
	fpmr	 23,0

loop2:
	fxcpnsma 0,24,28,0	# S0 * ~S2, phase 1
	fxcpnsma 1,24,29,1	# f1r += f29r*f24r, f1i -= f29i*f24r
	fxcpnsma 2,25,28,2
	fxcpnsma 3,25,29,3

	fxcpnsma 4,26,28,4	# S1 * ~S2, phase 1
	fxcpnsma 5,26,29,5
	fxcpnsma 6,27,28,6
	fxcpnsma 7,27,29,7

	fxcxma   0,24,28,0	# S0 * ~S2, phase 2
	fxcxma   1,24,29,1	# f1r += f29i*f24i, f1i += f29r*f24i
	fxcxma   2,25,28,2
	fxcxma   3,25,29,3

	fxcxma   4,26,28,4	# S1 * ~S2, phase 2
	fxcxma   5,26,29,5
	fxcxma   6,27,28,6
	fxcxma   7,27,29,7

	lfpsux	 28,7,11	# ld S4
	lfpsux	 29,7,11

	fxcpnsma 8,24,30,8	# S0 * ~S3, phase 1
	fxcpnsma 9,24,31,9
	fxcpnsma 10,25,30,10
	fxcpnsma 11,25,31,11

	fxcpnsma 12,26,30,12	# S1 * ~S3, phase 1
	fxcpnsma 13,26,31,13
	fxcpnsma 14,27,30,14
	fxcpnsma 15,27,31,15

	fxcxma   8,24,30,8	# S0 * ~S3, phase 2
	fxcxma   9,24,31,9
	fxcxma   10,25,30,10
	fxcxma   11,25,31,11

	fxcxma   12,26,30,12	# S1 * ~S3, phase 2
	fxcxma   13,26,31,13
	fxcxma   14,27,30,14
	fxcxma   15,27,31,15

	lfpsux	 30,5,11	# ld S2
	lfpsux	 31,5,11

	fxcpnsma 16,24,28,16	# S0 * ~S4, phase 1
	fxcpnsma 17,24,29,17	# f17r += f29r*f24r, f17i -= f29i*f24r
	fxcpnsma 18,25,28,18
	fxcpnsma 19,25,29,19

	fxcpnsma 20,26,28,20	# S1 * ~S4, phase 1
	fxcpnsma 21,26,29,21
	fxcpnsma 22,27,28,22
	fxcpnsma 23,27,29,23

	fxcxma   16,24,28,16	# S0 * ~S4, phase 2
	fxcxma   17,24,29,17	# f17r += f29i*f24i, f17i += f29r*f24i
	fxcxma   18,25,28,18
	fxcxma   19,25,29,19

	fxcxma   20,26,28,20	# S1 * ~S4, phase 2
	fxcxma   21,26,29,21
	fxcxma   22,27,28,22
	fxcxma   23,27,29,23

	lfpsux	 24,3,11	# ld S0
	lfpsux	 25,3,11
	lfpsux	 26,4,11	# ld S1
	lfpsux	 27,4,11
	lfpsux	 28,6,11	# ld S3
	lfpsux	 29,6,11

	fxcpnsma 0,24,30,0	# S0 * ~S2, phase 1
	fxcpnsma 1,24,31,1	# f1r += f31r*f24r, f1i -= f31i*f24r
	fxcpnsma 2,25,30,2
	fxcpnsma 3,25,31,3

	fxcpnsma 4,26,30,4	# S1 * ~S2, phase 1
	fxcpnsma 5,26,31,5
	fxcpnsma 6,27,30,6
	fxcpnsma 7,27,31,7

	fxcxma   0,24,30,0	# S0 * ~S2, phase 2
	fxcxma   1,24,31,1	# f1r += f31i*f24i, f1i += f31r*f24i
	fxcxma   2,25,30,2
	fxcxma   3,25,31,3

	fxcxma   4,26,30,4	# S1 * ~S2, phase 2
	fxcxma   5,26,31,5
	fxcxma   6,27,30,6
	fxcxma   7,27,31,7

	lfpsux	 30,7,11	# ld S4
	lfpsux	 31,7,11

	fxcpnsma 8,24,28,8	# S0 * ~S3, phase 1
	fxcpnsma 9,24,29,9
	fxcpnsma 10,25,28,10
	fxcpnsma 11,25,29,11

	fxcpnsma 12,26,28,12	# S1 * ~S3, phase 1
	fxcpnsma 13,26,29,13
	fxcpnsma 14,27,28,14
	fxcpnsma 15,27,29,15

	fxcxma   8,24,28,8	# S0 * ~S3, phase 2
	fxcxma   9,24,29,9
	fxcxma   10,25,28,10
	fxcxma   11,25,29,11

	fxcxma   12,26,28,12	# S1 * ~S3, phase 2
	fxcxma   13,26,29,13
	fxcxma   14,27,28,14
	fxcxma   15,27,29,15

	lfpsux	 28,5,11	# ld S2
	lfpsux	 29,5,11

	dcbt	 3,11		# fetch 2 station S0 samples from L2 to L1 cache
	dcbt	 4,11		# ditto for S1

	fxcpnsma 16,24,30,16	# S0 * ~S4, phase 1
	fxcpnsma 17,24,31,17	# f17r += f31r*f24r, f17i -= f31i*f24r
	fxcpnsma 18,25,30,18
	fxcpnsma 19,25,31,19

	fxcpnsma 20,26,30,20	# S1 * ~S4, phase 1
	fxcpnsma 21,26,31,21
	fxcpnsma 22,27,30,22
	fxcpnsma 23,27,31,23

	fxcxma   16,24,30,16	# S0 * ~S4, phase 2
	fxcxma   17,24,31,17	# f17r += f31i*f24i, f17i += f31r*f24i
	fxcxma   18,25,30,18
	fxcxma   19,25,31,19

	fxcxma   20,26,30,20	# S1 * ~S4, phase 2
	fxcxma   21,26,31,21
	fxcxma   22,27,30,22
	fxcxma   23,27,31,23

	lfpsux	 24,3,11	# ld S0
	lfpsux	 25,3,11
	lfpsux	 26,4,11	# ld S1
	lfpsux	 27,4,11
	lfpsux	 30,6,11	# ld S3
	lfpsux	 31,6,11

	bdnz	 loop2

	lwz	 3,288+8(1)	# load function arguments 9-11
	lwz	 4,288+12(1)
	lwz	 5,288+16(1)

	dcbz	 0,8
	stfpsx	 0,0,8		# store results S0 * ~S2
	stfpsux	 1,8,11
	stfpsux	 2,8,11
	stfpsux	 3,8,11

	dcbz	 0,9
	stfpsx	 4,0,9		# store results S1 * ~S2
	stfpsux	 5,9,11
	stfpsux	 6,9,11
	stfpsux	 7,9,11

	dcbz	 0,10
	stfpsx	 8,0,10		# store results S0 * ~S3
	stfpsux	 9,10,11
	stfpsux	 10,10,11
	stfpsux	 11,10,11

	dcbz	 0,3
	stfpsx	 12,0,3		# store results S1 * ~S3
	stfpsux	 13,3,11
	stfpsux	 14,3,11
	stfpsux	 15,3,11

	dcbz	 0,4
	stfpsx	 16,0,4		# store results S0 * ~S4
	stfpsux	 17,4,11
	stfpsux	 18,4,11
	stfpsux	 19,4,11

	dcbz	 0,5
	stfpsx	 20,0,5		# store results S1 * ~S4
	stfpsux	 21,5,11
	stfpsux	 22,5,11
	stfpsux	 23,5,11

	li	 11,16		# restore call-saved registers
	lfpdx	 31,0,1
	lfpdux	 30,1,11
	lfpdux	 29,1,11
	lfpdux	 28,1,11
	lfpdux	 27,1,11
	lfpdux	 26,1,11
	lfpdux	 25,1,11
	lfpdux	 24,1,11
	lfpdux	 23,1,11
	lfpdux	 22,1,11
	lfpdux	 21,1,11
	lfpdux	 20,1,11
	lfpdux	 19,1,11
	lfpdux	 18,1,11
	lfpdux	 17,1,11
	lfpdux	 16,1,11
	lfpdux	 15,1,11
	lfpdux	 14,1,11

	addi	 1,1,16		# reset stack pointer

	blr			# return



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

	lis	 12,zero@ha
	addi	 12,12,zero@l
	lfpsx	 0,0,12

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

	fpmr	 1,0
	fpmr	 2,0
	fpmr	 3,0
	fpmr	 4,0
	fpmr	 5,0
	fpmr	 6,0
	fpmr	 7,0
	fpmr	 8,0
	fpmr	 9,0
	fpmr	 10,0
	fpmr	 11,0

loop3:	# loop over time

	lfpsux	 16,3,11	# read station samples
	lfpsux	 17,3,11
	lfpsux	 18,4,11
	lfpsux	 19,4,11

	fpmadd	 0,12,12,0	# S0_X * ~S0_X, phase 1
	fxcpnsma 1,12,13,1	# S0_X * ~S0_Y, phase 1
	fpmadd	 3,13,13,3	# S0_Y * ~S0_Y, phase 1

	fxcpnsma 4,12,14,4	# S0 * ~S1, phase 1
	fxcpnsma 5,12,15,5
	fxcpnsma 6,13,14,6
	fxcpnsma 7,13,15,7

	fpmadd	 8,14,14,8	# S1 * ~S1, phase 1
	fxcpnsma 9,14,15,9
	fpmadd	 11,15,15,11

	fxcxma   1,12,13,1	# S0_X * ~S0_Y, phase 2

	fxcxma   4,12,14,4	# S0 * ~S1, phase 2
	fxcxma   5,12,15,5
	fxcxma   6,13,14,6
	fxcxma   7,13,15,7

	fxcxma   9,14,15,9	# S1 * ~S1, phase 2

	lfpsux	 12,3,11	# f12:f13 = S0
	lfpsux	 13,3,11
	lfpsux	 14,4,11	# f14:f15 = S1
	lfpsux	 15,4,11

	fpmadd	 0,16,16,0	# S0 * ~S0, phase 1
	fxcpnsma 1,16,17,1	# f1r += f17r*f16r, f1i -= f17i*f16r
	fpmadd	 3,17,17,3

	fxcpnsma 4,16,18,4	# S0 * ~S1, phase 1
	fxcpnsma 5,16,19,5
	fxcpnsma 6,17,18,6
	fxcpnsma 7,17,19,7

	fpmadd	 8,18,18,8	# S1 * ~S1, phase 1
	fxcpnsma 9,18,19,9
	fpmadd	 11,19,19,11

	fxcxma   1,16,17,1	# f1r += f17i*f16i, f1i += f17r*f16i

	fxcxma   4,16,18,4	# S0 * ~S1, phase 2
	fxcxma   5,16,19,5
	fxcxma   6,17,18,6
	fxcxma   7,17,19,7

	fxcxma   9,18,19,9	# S1 * ~S1, phase 2

	bdnz	 loop3

	lfpsx	 12,12,11	# f12r = 1, f12i = 1

	fxcsnsma 0,0,12,0	# f0r += 1*f0i, f0i += -1*f0i = 0
	fxcsnsma 3,3,12,3
	fxcsnsma 8,8,12,8
	fxcsnsma 11,11,12,11

	dcbz	 0,5
	stfpsx	 0,0,5		# store results S0 * ~S0
	stfpsux	 1,5,11
	fsneg	 1,1
	stfpsux	 1,5,11
	stfpsux	 3,5,11

	dcbz	 0,6
	stfpsx	 4,0,6		# store results S0 * ~S1
	stfpsux	 5,6,11
	stfpsux	 6,6,11
	stfpsux	 7,6,11

	dcbz	 0,7
	stfpsx	 8,0,7		# store results S1 * ~S1
	stfpsux	 9,7,11
	fsneg	 9,9
	stfpsux	 9,7,11
	stfpsux	 11,7,11

	li	  11,16		# restore call-saved registers
	lfpdx	  19,0,1
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

	li	 5,NR_SAMPLES_PER_INTEGRATION/4
	mtctr	 5

	lis	 12,zero@ha
	addi	 12,12,zero@l
	lfpsx	 0,0,12		# f0r = 0, f0i = 0

	li	 8,8

	lfpsx	 4,0,3		# f8:f9 = S0
	lfpsux	 5,3,8
	lfpsux	 6,3,8		# f6:f7 = S0
	lfpsux	 7,3,8
	lfpsux	 8,3,8		# f8:f9 = S0
	lfpsux	 9,3,8

	lfpsx	 12,12,8	# f12r = 1, f12i = 1

	fpmr	 1,0
	fpmr	 2,0
	fpmr	 3,0

loop3:	# loop over time

	lfpsux	 10,3,8		# f6:f7 = S0
	lfpsux	 11,3,8

	fpmadd	 0,4,4,0	# S0 * ~S0
	fxcpnsma 1,4,5,1	# f1r += f5r*f4r, f1i -= f5i*f4r
	fxcxma   2,4,5,2	# f2r += f5i*f4i, f2i += f5r*f4i
	fpmadd	 3,5,5,3

	lfpsux	 4,3,8		# f4:f5 = S0
	lfpsux	 5,3,8

	fpmadd	 0,6,6,0	# S0 * ~S0
	fxcpnsma 1,6,7,1	# f1r += f7r*f6r, f1i -= f7i*f6r
	fxcxma   2,6,7,2	# f2r += f7i*f6i, f2i += f7r*f6i
	fpmadd	 3,7,7,3

	lfpsux	 6,3,8		# f6:f7 = S0
	lfpsux	 7,3,8

	fpmadd	 0,8,8,0	# S0 * ~S0
	fxcpnsma 1,8,9,1	# f1r += f9r*f8r, f1i -= f9i*f8r
	fxcxma   2,8,9,2	# f2r += f9i*f8i, f2i += f9r*f8i
	fpmadd	 3,9,9,3

	lfpsux	 8,3,8		# f8:f9 = S0
	lfpsux	 9,3,8

	fpmadd	 0,10,10,0	# S0 * ~S0
	fxcpnsma 1,10,11,1	# f1r += f11r*f10r, f1i -= f11i*f10r
	fxcxma   2,10,11,2	# f2r += f11i*f10i, f2i += f11r*f10i
	fpmadd	 3,11,11,3

	bdnz	 loop3

	fxcsnsma 0,0,12,0	# f0r += 1*f0i, f0i += -1*f0i = 0
	fxcsnsma 3,3,12,3
	fpadd	 1,1,2

	dcbz	 0,4		# store result
	stfpsux	 0,0,4
	stfpsux	 1,4,8
	fsneg	 1,1
	stfpsux	 1,4,8
	stfpsux	 3,4,8

	blr			# return

#endif
#endif
