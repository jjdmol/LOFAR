#if defined HAVE_BGL
#include <TFC_Interface/TFC_Config.h>

#define I16COMPLEX_SIZE	 4
#define FCOMPLEX_SIZE	 8
#define DCOMPLEX_SIZE	16


.align	5
.global _transpose_8x4
_transpose_8x4:

	li	0,NR_SUB_CHANNELS*NR_POLARIZATIONS
	mtctr	0

	subi	4,4,I16COMPLEX_SIZE

loop1:	
	lwzu	5,I16COMPLEX_SIZE(4)
	lwz	6,1*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)
	lwz	7,2*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)
	lwz	8,3*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)
	lwz	9,4*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)
	lwz	10,5*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)
	lwz	11,6*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)
	lwz	12,7*I16COMPLEX_SIZE*NR_SUB_CHANNELS*NR_POLARIZATIONS(4)

	dcbz	0,3
	stw	5,0(3)
	stw	6,4(3)
	stw	7,8(3)
	stw	8,12(3)
	stw	9,16(3)
	stw	10,20(3)
	stw	11,24(3)
	stw	12,28(3)
#if I16COMPLEX_SIZE*NR_SAMPLES_PER_INTEGRATION < 32768
	addi	3,3,I16COMPLEX_SIZE*NR_SAMPLES_PER_INTEGRATION
#else
	addis	3,3,I16COMPLEX_SIZE*NR_SAMPLES_PER_INTEGRATION / 65536
	addi	3,3,I16COMPLEX_SIZE*NR_SAMPLES_PER_INTEGRATION & 0xFFFF
#endif
	bdnz	loop1

	blr


.align	5
.global	_transpose_4x8
_transpose_4x8:

	mulli	8,6,3
	mtctr	5
	subi	7,7,3*FCOMPLEX_SIZE
	li	9,8
	subfic	8,8,FCOMPLEX_SIZE

	sub	3,3,7
	sub	4,4,8

L4:	
	lfpsux	0,4,8
	lfpsux	1,4,6
	lfpsux	2,4,6
	lfpsux	3,4,6

	dcbz	3,7
	stfpsux	0,3,7
	stfpsux	1,3,9
	stfpsux	2,3,9
	stfpsux	3,3,9

	bdnz	L4

	blr


.align	5
.global	_phase_shift_and_transpose
_phase_shift_and_transpose:

#	Performs partial transpose between FFT and correlator and multiplies
#	phase shifts on the fly.
#	The outputs of four FFTs are transposed: two time slots for each of
#	the two	polarizations.

#	arguments:
#	r3 :	fcomplex *out
#	r4 :	const fcomplex *in
#	r5 :	const dcomplex phases[2][2]: v0, delta_v0, v1, delta_v1

#	f8 :	v0;
#	f9 :	delta_v0
#	f10 :	v1
#	f11 :	delta_v1

	li	9,-16		# save registers
	stfpdux 14,1,9
	stfpdux 15,1,9
	stfpdux 16,1,9
	stfpdux 17,1,9

	lfpdx	8,0,5
	li	9,16
	lfpdux	9,5,9
	lfpdux	10,5,9
	lfpdux	11,5,9

	li	0,NR_SUB_CHANNELS/2
	li	6,FCOMPLEX_SIZE*NR_SUB_CHANNELS
	lis	7,FCOMPLEX_SIZE*(NR_POLARIZATIONS*NR_SAMPLES_PER_INTEGRATION*NR_STATIONS-3)@ha
	addi	7,7,FCOMPLEX_SIZE*(NR_POLARIZATIONS*NR_SAMPLES_PER_INTEGRATION*NR_STATIONS-3)@l
	li	8,FCOMPLEX_SIZE*(1-3*NR_SUB_CHANNELS)
	li	10,8
	mtctr	0

	sub	3,3,7
	sub	4,4,8

	lfpsux	0,4,8
	lfpsux	1,4,6
	lfpsux	2,4,6
	lfpsux	3,4,6

L5:	
	lfpsux	4,4,8
	lfpsux	5,4,6
	lfpsux	6,4,6
	lfpsux	7,4,6

	fxpmul	12,0,8
	fxpmul	13,1,8
	fxpmul	14,2,10
	fxpmul	15,3,10

	fxpmul  16,8,9
	fxpmul  17,10,11

	fxcxnpma 0,0,8,12
	fxcxnpma 1,1,8,13
	fxcxnpma 2,2,10,14
	fxcxnpma 3,3,10,15

	fxcxnpma 8,8,9,16
	fxcxnpma 10,10,11,17
 
	dcbz	3,7
	stfpsux	0,3,7
	stfpsux	1,3,10
	stfpsux	2,3,10
	stfpsux	3,3,10

	lfpsux	0,4,8
	lfpsux	1,4,6
	lfpsux	2,4,6
	lfpsux	3,4,6

	fxpmul	12,4,8
	fxpmul	13,5,8
	fxpmul	14,6,10
	fxpmul	15,7,10

	fxpmul  16,8,9
	fxpmul  17,10,11

	fxcxnpma 4,4,8,12
	fxcxnpma 5,5,8,13
	fxcxnpma 6,6,10,14
	fxcxnpma 7,7,10,15

	fxcxnpma 8,8,9,16
	fxcxnpma 10,10,11,17
 
	dcbz	3,7
	stfpsux	4,3,7
	stfpsux	5,3,10
	stfpsux	6,3,10
	stfpsux	7,3,10

	bdnz	L5

	lfpdx	17,0,1		# restore registers
	lfpdux	16,1,9
	lfpdux	15,1,9
	lfpdux	14,1,9
	addi	1,1,16

	blr


.align	5
sub_value:
	.long	 0x43300000,0x00008000,0x43300000,0x00008000


.global	_filter
_filter:
#	filters all samples for one station, one polarization

#	arguments:
#	r3 :	  pointer to delay line (fcomplex[16])
#	r4 :	  pointer to weights line (const fcomplex[16])
#	r5 :	  pointer to first sample (const i16complex[16*r7])
#	r6 :	  pointer to result (fcomplex *)
#	r7 :	  number of samples / 16

#	internally used:
#	r9 :	  8
#	r10 :	  2048
#	r11 :	  8
#	r12 :	  0x80008000
#	r16-r31 : heavily prefetched samples
#	f0-f15 :  delay line (real in primary, imaginary in secondary unit)
#	f16-f23 : weights (these are real values alternately stored in primary
#			   and secondary units)
#	f24-f29 : sums
#	f31 :	  sub_value

#	The implementation works on 5 or 6 time samples concurrently, to avoid
#	stalls in the double hummer.  This unfortunately leads to totally
#	incomprehensible code.  The loop processes 16 samples at a time.
#	The input is converted from int16complex to dcomplex by black magic,
#	making the code even harder to understand.

	stwu	1,-368(1)	# adjust stack pointer

	la	8,0(1)		# save call-saved registers
	li	9,DCOMPLEX_SIZE
	stfpdux 14,8,9
	stfpdux 15,8,9
	stfpdux 16,8,9
	stfpdux 17,8,9
	stfpdux 18,8,9
	stfpdux 19,8,9
	stfpdux 20,8,9
	stfpdux 21,8,9
	stfpdux 22,8,9
	stfpdux 23,8,9
	stfpdux 24,8,9
	stfpdux 25,8,9
	stfpdux 26,8,9
	stfpdux 27,8,9
	stfpdux 28,8,9
	stfpdux 29,8,9
#	stfpdux 30,8,9		# f30 not used at the moment
	stfpdux 31,8,9
	stmw	16,304(1)	# save r16 ... r31

	lis	8,sub_value@ha	# load sub_values
	la	8,sub_value@l(8)
	lfpdx	31,0,8

	li	9,FCOMPLEX_SIZE
	#li	10,I16COMPLEX_SIZE*(NR_SUB_CHANNELS*NR_POLARIZATIONS)
	li	10,I16COMPLEX_SIZE
	li	11,FCOMPLEX_SIZE
	lis	12,0x8000
	ori	12,12,0x8000

	sub	6,6,11

	stfpdx	31,0,1		# initialize int->fp conversion area

	lwzx	16,0,5		# prefetch samples before entering loop
	lwzux	17,5,10
	lwzux	18,5,10
	lwzux	19,5,10
	lwzux	20,5,10
	lwzux	21,5,10
	lwzux	22,5,10
	lwzux	23,5,10
	lwzux	24,5,10
	lwzux	25,5,10
	lwzux	26,5,10
	lwzux	27,5,10
	lwzux	28,5,10
	lwzux	29,5,10
	lwzux	30,5,10
	lwzux	31,5,10

	lfpsx	1,0,3		# load delay line
	lfpsux	2,3,9
	lfpsux	3,3,9
	lfpsux	4,3,9
	lfpsux	5,3,9
	lfpsux	6,3,9
	lfpsux	7,3,9
	lfpsux	8,3,9
	lfpsux	9,3,9
	lfpsux	10,3,9
	lfpsux	11,3,9
	lfpsux	12,3,9
	lfpsux	13,3,9
	lfpsux	14,3,9
	lfpsux	15,3,9

	lfpsx	16,0,4		# load weights line
	lfpsux	17,4,9
	lfpsux	18,4,9
	lfpsux	19,4,9
	lfpsux	20,4,9
	lfpsux	21,4,9
	lfpsux	22,4,9
	lfpsux	23,4,9

	mtctr	7		# set number of iterations

loop:
	# time steps 0-5
	xor	16,16,12
	sth	16,14(1)
	srawi	16,16,16
	sth	16,6(1)
	fxpmul	 24,20,8
	fxsmul	 25,20,8
	lfpdx	0,0,1
	fxpmul	 26,21,8
	fxsmul	 27,21,8
	fxpmul	 28,22,8
	fxsmul	 29,22,8
	fpsub	0,0,31
	lwzux	16,5,10
	fxcpmadd 24,19,10,24
	fxcsmadd 25,19,10,25
	fxcpmadd 26,20,10,26
	fxcsmadd 27,20,10,27
	xor	17,17,12
	fxcpmadd 28,21,10,28
	sth	17,14(1)
	fxcsmadd 29,21,10,29
	fxcpmadd 24,16,0,24
	fxcsmadd 25,16,0,25
	srawi	17,17,16
	fxcpmadd 26,17,0,26
	sth	17,6(1)
	fxcsmadd 27,17,0,27
	fxcpmadd 28,18,0,28
	fxcsmadd 29,18,0,29

	fxcsmadd 24,23,1,24
	lfpdx	1,0,1
	fxcpmadd 25,20,9,25
	xor	18,18,12
	fxcsmadd 26,20,9,26
	sth	18,14(1)
	fxcpmadd 27,21,9,27
	fxcsmadd 28,21,9,28
	lwzux	17,5,10
	fxcpmadd 29,22,9,29
	srawi	18,18,16
	fpsub	1,1,31
	sth	18,6(1)

	fxcpmadd 24,23,2,24
	fxcsmadd 25,23,2,25
	lfpdx	2,0,1
	fxcpmadd 26,22,6,26
	fxcsmadd 27,22,6,27
	fxcpmadd 28,23,6,28
	fxcsmadd 29,23,6,29
	lwzux	18,5,10
	fxcpmadd 24,21,6,24
	fxcsmadd 25,21,6,25
	fpsub	2,2,31
	fxcsmadd 26,16,1,26
	fxcpmadd 27,17,1,27
	fxcsmadd 28,17,1,28
	fxcpmadd 29,18,1,29
	xor	19,19,12
	fxcsmadd 24,19,9,24
	sth	19,14(1)
	fxcpmadd 25,16,1,25
	fxcpmadd 26,16,2,26
	fxcsmadd 27,16,2,27
	srawi	19,19,16
	fxcpmadd 28,17,2,28
	sth	19,6(1)
	fxcsmadd 29,17,2,29

	fxcsmadd 24,22,3,24
	fxcpmadd 25,23,3,25
	fxcsmadd 26,23,3,26
	lfpdx	3,0,1
	fxcsmadd 27,18,14,27
	fxcpmadd 28,19,14,28
	fxcsmadd 29,19,14,29
	fxcpmadd 24,17,14,24
	fxcsmadd 25,17,14,25
	lwzux	19,5,10
	fxcpmadd 26,18,14,26
	fpsub	3,3,31
	fxcpmadd 27,20,11,27
	fxcsmadd 28,20,11,28
	fxcpmadd 29,21,11,29
	fxcsmadd 24,18,11,24
	xor	20,20,12
	fxcpmadd 25,19,11,25
	sth	20,14(1)
	fxcsmadd 26,19,11,26
	fxcpmadd 27,16,3,27
	fxcsmadd 28,16,3,28
	srawi	20,20,16
	fxcpmadd 29,17,3,29

	sth	20,6(1)
	fxcpmadd 24,22,4,24
	fxcsmadd 25,22,4,25
	fxcpmadd 26,23,4,26
	fxcsmadd 27,23,4,27
	lfpdx	4,0,1
	fxcsmadd 28,22,7,28
	fxcpmadd 29,23,7,29
	fxcsmadd 24,20,7,24
	fxcpmadd 25,21,7,25
	fxcsmadd 26,21,7,26
	lwzux	20,5,10
	fxcpmadd 27,22,7,27
	fpsub	4,4,31
	fxcpmadd 28,20,12,28
	fxcsmadd 29,20,12,29
	fxcpmadd 24,18,12,24
	fxcsmadd 25,18,12,25
	xor	21,21,12
	fxcpmadd 26,19,12,26
	sth	21,14(1)
	fxcsmadd 27,19,12,27
	fxcpmadd 28,16,4,28
	fxcsmadd 29,16,4,29

	srawi	21,21,16
	fxcsmadd 24,21,5,24
	sth	21,6(1)
	fxcpmadd 25,22,5,25
	fxcsmadd 26,22,5,26
	fxcpmadd 27,23,5,27
	fxcsmadd 28,23,5,28
	lfpdx	5,0,1
	fxcpmadd 29,19,15,29
	fxcsmadd 24,16,15,24
	fxcpmadd 25,17,15,25
	fxcsmadd 26,17,15,26
	lwzux	21,5,10
	fxcpmadd 27,18,15,27
	xor	22,22,12
	fxcsmadd 28,18,15,28
	sth	22,14(1)
	fpsub	5,5,31
	fxcpmadd 29,20,13,29
	fxcsmadd 24,17,13,24
	fxcpmadd 25,18,13,25
	srawi	22,22,16
	sth	22,6(1)
	fxcsmadd 26,18,13,26
	fxcpmadd 27,19,13,27
	fxcsmadd 28,19,13,28
	fxcpmadd 29,16,5,29

	lfpdx	6,0,1
	xor	23,23,12
	fpsub	6,6,31
	sth	23,14(1)

	srawi	23,23,16
	sth	23,6(1)

	lwzux	22,5,10

	# time steps 6-10
	stfpsux	 24,6,11		# (should be interleaved as well)
	fxsmul	 24,23,7
	lfpdx	7,0,1
	stfpsux	 25,6,11
	fxpmul	 25,20,15
	stfpsux	 26,6,11
	fxsmul	 26,20,15
	stfpsux	 27,6,11
	fxpmul	 27,21,15
	stfpsux	 28,6,11
	fxsmul	 28,21,15
	fxcsmadd 24,19,15,24
	fpsub	7,7,31
	stfpsux	 29,6,11
	fxcsmadd 25,17,4,25
	fxcpmadd 26,18,4,26
	xor	24,24,12
	fxcsmadd 27,18,4,27
	sth	24,14(1)
	fxcpmadd 28,19,4,28
	fxcpmadd 24,17,4,24
	lwzux	23,5,10
	fxcpmadd 25,16,7,25
	srawi	24,24,16
	fxcsmadd 26,16,7,26
	sth	24,6(1)
	fxcpmadd 27,17,7,27
	fxcsmadd 28,17,7,28

	fxcpmadd 24,23,8,24
	fxcsmadd 25,23,8,25
	lfpdx	8,0,1
	fxcsmadd 26,21,13,26
	fxcpmadd 27,22,13,27
	fxcsmadd 28,22,13,28
	fxcsmadd 24,20,13,24
	lwzux	24,5,10
	fxcpmadd 25,21,13,25
	fpsub	8,8,31
	fxcpmadd 26,20,0,26
	fxcsmadd 27,20,0,27
	xor	25,25,12
	fxcpmadd 28,21,0,28
	sth	25,14(1)
	fxcpmadd 24,19,0,24
	fxcsmadd 25,19,0,25
	fxcpmadd 26,16,8,26
	srawi	25,25,16
	fxcsmadd 27,16,8,27
	sth	25,6(1)
	fxcpmadd 28,17,8,28

	fxcsmadd 24,22,9,24
	fxcpmadd 25,23,9,25
	fxcsmadd 26,23,9,26
	lfpdx	9,0,1
	fxcpmadd 27,18,5,27
	fxcsmadd 28,18,5,28
	fxcsmadd 24,16,5,24
	fxcpmadd 25,17,5,25
	lwzux	25,5,10
	fxcsmadd 26,17,5,26
	fpsub	9,9,31
	fxcpmadd 27,20,1,27
	fxcsmadd 28,20,1,28
	xor	26,26,12
	fxcsmadd 24,18,1,24
	sth	26,14(1)
	fxcpmadd 25,19,1,25
	fxcsmadd 26,19,1,26
	fxcpmadd 27,16,9,27
	srawi	26,26,16
	fxcsmadd 28,16,9,28

	sth	26,6(1)
	fxcpmadd 24,22,10,24
	fxcsmadd 25,22,10,25
	fxcpmadd 26,23,10,26
	fxcsmadd 27,23,10,27
	lfpdx	10,0,1
	fxcpmadd 28,22,14,28
	fxcpmadd 24,20,14,24
	fxcsmadd 25,20,14,25
	fxcpmadd 26,21,14,26
	lwzux	26,5,10
	fxcsmadd 27,21,14,27
	fpsub	10,10,31
	fxcpmadd 28,20,2,28
	fxcpmadd 24,18,2,24
	fxcsmadd 25,18,2,25
	fxcpmadd 26,19,2,26
	fxcsmadd 27,19,2,27
	fxcpmadd 28,16,10,28

	fxcsmadd 24,21,11,24
	xor	27,27,12
	fxcpmadd 25,22,11,25
	sth	27,14(1)
	fxcsmadd 26,22,11,26
	fxcpmadd 27,23,11,27
	fxcsmadd 28,23,11,28
	srawi	27,27,16
	fxcsmadd 24,17,3,24
	sth	27,6(1)
	fxcpmadd 25,18,3,25
	fxcsmadd 26,18,3,26
	fxcpmadd 27,19,3,27
	fxcsmadd 28,19,3,28

	lfpdx	11,0,1
	fxcpmadd 24,16,6,24
	fxcsmadd 25,16,6,25
	fxcpmadd 26,17,6,26
	fxcsmadd 27,17,6,27
	lwzux	27,5,10
	fxcpmadd 28,18,6,28
	fpsub	11,11,31

	fxcpmadd 24,21,12,24
	fxcsmadd 25,21,12,25
	fxcpmadd 26,22,12,26
	fxcsmadd 27,22,12,27
	fxcpmadd 28,23,12,28


	# time steps 11-15
	stfpsux	 24,6,11
	fxsmul	 24,21,0
	stfpsux	 25,6,11
	fxpmul	 25,22,0
	stfpsux	 26,6,11
	fxsmul	 26,22,0
	stfpsux	 27,6,11
	fxpmul	 27,23,0
	stfpsux	 28,6,11
	fxsmul	 28,23,0
	fxcsmadd 24,17,8,24
	fxcpmadd 25,18,8,25
	fxcsmadd 26,18,8,26
	fxcpmadd 27,19,8,27
	fxcsmadd 28,19,8,28

	fxcpmadd 24,21,1,24
	fxcsmadd 25,21,1,25
	xor	28,28,12
	fxcpmadd 26,22,1,26
	sth	28,14(1)
	fxcsmadd 27,22,1,27
	fxcpmadd 28,23,1,28
	fxcpmadd 24,17,9,24
	srawi	28,28,16
	fxcsmadd 25,17,9,25
	sth	28,6(1)
	fxcpmadd 26,18,9,26
	fxcsmadd 27,18,9,27
	fxcpmadd 28,19,9,28

	fxcsmadd 24,23,12,24
	lfpdx	12,0,1
	fxcpmadd 25,21,2,25
	fxcsmadd 26,21,2,26
	fxcpmadd 27,22,2,27
	fxcsmadd 28,22,2,28
	lwzux	28,5,10
	fxcsmadd 24,20,2,24
	fpsub	12,12,31
	fxcpmadd 25,20,4,25
	fxcsmadd 26,20,4,26
	xor	29,29,12
	fxcpmadd 27,21,4,27
	sth	29,14(1)
	fxcsmadd 28,21,4,28
	fxcsmadd 24,19,4,24
	fxcpmadd 25,16,12,25
	srawi	29,29,16
	fxcsmadd 26,16,12,26
	sth	29,6(1)
	fxcpmadd 27,17,12,27
	fxcsmadd 28,17,12,28

	fxcpmadd 24,23,13,24
	fxcsmadd 25,23,13,25
	lfpdx	13,0,1
	fxcsmadd 26,17,10,26
	fxcpmadd 27,18,10,27
	fxcsmadd 28,18,10,28
	fxcsmadd 24,16,10,24
	lwzux	29,5,10
	fxcpmadd 25,17,10,25
	fpsub	13,13,31
	fxcpmadd 26,20,5,26
	fxcsmadd 27,20,5,27
	xor	30,30,12
	fxcpmadd 28,21,5,28
	sth	30,14(1)
	fxcpmadd 24,19,5,24
	fxcsmadd 25,19,5,25
	fxcpmadd 26,16,13,26
	srawi	30,30,16
	fxcsmadd 27,16,13,27
	sth	30,6(1)
	fxcpmadd 28,17,13,28

	fxcsmadd 24,22,14,24
	fxcpmadd 25,23,14,25
	fxcsmadd 26,23,14,26
	lfpdx	14,0,1
	fxcsmadd 27,21,3,27
	fxcpmadd 28,22,3,28
	fxcpmadd 24,20,3,24
	fxcsmadd 25,20,3,25
	lwzux	30,5,10
	fxcpmadd 26,21,3,26
	fpsub	14,14,31
	fxcpmadd 27,20,6,27
	fxcsmadd 28,20,6,28
	xor	31,31,12
	fxcsmadd 24,18,6,24
	sth	31,14(1)
	fxcpmadd 25,19,6,25
	fxcsmadd 26,19,6,26
	fxcpmadd 27,16,14,27
	srawi	31,31,16
	fxcsmadd 28,16,14,28

	sth	31,6(1)
	fxcpmadd 24,22,15,24
	fxcsmadd 25,22,15,25
	fxcpmadd 26,23,15,26
	fxcsmadd 27,23,15,27
	lfpdx	15,0,1
	fxcpmadd 28,18,11,28
	fxcpmadd 24,16,11,24
	fxcsmadd 25,16,11,25
	fxcpmadd 26,17,11,26
	lwzux	31,5,10
	fxcsmadd 27,17,11,27
	fpsub	15,15,31
	fxcpmadd 28,20,7,28
	fxcpmadd 24,18,7,24
	fxcsmadd 25,18,7,25
	fxcpmadd 26,19,7,26
	fxcsmadd 27,19,7,27
	fxcpmadd 28,16,15,28

	stfpsux	 24,6,11
	stfpsux	 25,6,11
	stfpsux	 26,6,11
	stfpsux	 27,6,11
	stfpsux	 28,6,11


	bdnz	loop

	addi	3,3,-120
	stfpsux	1,3,9		# store delay line
	stfpsux	2,3,9
	stfpsux	3,3,9
	stfpsux	4,3,9
	stfpsux	5,3,9
	stfpsux	6,3,9
	stfpsux	7,3,9
	stfpsux	8,3,9
	stfpsux	9,3,9
	stfpsux	10,3,9
	stfpsux	11,3,9
	stfpsux	12,3,9
	stfpsux	13,3,9
	stfpsux	14,3,9
	stfpsux	15,3,9

	la	8,-16(1)	# restore call-saved registers
	li	9,DCOMPLEX_SIZE

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
#	lfpdux	30,8,9
	lfpdux	31,8,9
	lmw	16,304(1)	# restore r16 ... r31

	addi	1,1,368		# restore stack pointer
	blr			# return



.align	5
.global	_fast_memcpy
_fast_memcpy:

	li	8,-16
	stfpdux 14,1,8
	stfpdux 15,1,8
	stfpdux 16,1,8
	stfpdux 17,1,8
	stfpdux 18,1,8
	stfpdux 19,1,8
	stfpdux 20,1,8
	stfpdux 21,1,8
	stfpdux 22,1,8
	stfpdux 23,1,8
	stfpdux 24,1,8
	stfpdux 25,1,8
	stfpdux 26,1,8
	stfpdux 27,1,8
	stfpdux 28,1,8
	stfpdux 29,1,8
	stfpdux 30,1,8
	stfpdux 31,1,8

	srwi	5,5,9
	mtctr	5
	li	8,16

	sub	3,3,8
	sub	4,4,8

L0:
	lfpdux	0,4,8
	lfpdux	1,4,8
	lfpdux	2,4,8
	lfpdux	3,4,8
	lfpdux	4,4,8
	lfpdux	5,4,8
	lfpdux	6,4,8
	lfpdux	7,4,8
	lfpdux	8,4,8
	lfpdux	9,4,8
	lfpdux	10,4,8
	lfpdux	11,4,8
	lfpdux	12,4,8
	lfpdux	13,4,8
	lfpdux	14,4,8
	lfpdux	15,4,8
	lfpdux	16,4,8
	lfpdux	17,4,8
	lfpdux	18,4,8
	lfpdux	19,4,8
	lfpdux	20,4,8
	lfpdux	21,4,8
	lfpdux	22,4,8
	lfpdux	23,4,8
	lfpdux	24,4,8
	lfpdux	25,4,8
	lfpdux	26,4,8
	lfpdux	27,4,8
	lfpdux	28,4,8
	lfpdux	29,4,8
	lfpdux	30,4,8
	lfpdux	31,4,8
	stfpdux	0,3,8
	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	stfpdux	4,3,8
	stfpdux	5,3,8
	stfpdux	6,3,8
	stfpdux	7,3,8
	stfpdux	8,3,8
	stfpdux	9,3,8
	stfpdux	10,3,8
	stfpdux	11,3,8
	stfpdux	12,3,8
	stfpdux	13,3,8
	stfpdux	14,3,8
	stfpdux	15,3,8
	stfpdux	16,3,8
	stfpdux	17,3,8
	stfpdux	18,3,8
	stfpdux	19,3,8
	stfpdux	20,3,8
	stfpdux	21,3,8
	stfpdux	22,3,8
	stfpdux	23,3,8
	stfpdux	24,3,8
	stfpdux	25,3,8
	stfpdux	26,3,8
	stfpdux	27,3,8
	stfpdux	28,3,8
	stfpdux	29,3,8
	stfpdux	30,3,8
	stfpdux	31,3,8
	bdnz	L0

	lfpdx	31,0,1
	lfpdux	30,1,8
	lfpdux	29,1,8
	lfpdux	28,1,8
	lfpdux	27,1,8
	lfpdux	26,1,8
	lfpdux	25,1,8
	lfpdux	24,1,8
	lfpdux	23,1,8
	lfpdux	22,1,8
	lfpdux	21,1,8
	lfpdux	20,1,8
	lfpdux	19,1,8
	lfpdux	18,1,8
	lfpdux	17,1,8
	lfpdux	16,1,8
	lfpdux	15,1,8
	lfpdux	14,1,8
	addi	1,1,16
	blr
#endif
