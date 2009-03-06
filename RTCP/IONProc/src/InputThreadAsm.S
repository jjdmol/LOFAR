#  InputThreadAsm.S: fast packet->BBuffer copy routines
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  $Id$

#if defined HAVE_BGL || defined HAVE_BGP

.global	_copy_pkt_to_bbuffer_32_bytes
_copy_pkt_to_bbuffer_32_bytes:

	srwi	7,6,1
	andi.	0,6,1
	cmpwi	1,7,1
	subi	7,7,1

	sub	3,3,4
	addi	4,4,-1*16
	subi	5,5,16
	li	8,16

	blt	1,2f	# branch if nrSubbands < 2

	lfpdux	0,5,8	# prologue
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8

	beq	1,1f	# skip loop if nrSubbands < 4
	mtctr	7

0:	stfpdux	0,3,4	# unroll twice
	stfpdux	1,3,8
	lfpdux	0,5,8
	lfpdux	1,5,8
	stfpdux	2,3,4
	stfpdux	3,3,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	bdnz	0b

1:	stfpdux	0,3,4	# epilogue
	stfpdux	1,3,8
	stfpdux	2,3,4
	stfpdux	3,3,8

2:	beqlr	0

	lfpdux	0,5,8	# last subband, in case nrSubbands is uneven
	lfpdux	1,5,8
	stfpdux	0,3,4
	stfpdux	1,3,8

	blr


.global	_copy_pkt_to_bbuffer_64_bytes
_copy_pkt_to_bbuffer_64_bytes:

	srwi	7,6,1
	andi.	0,6,1
	cmpwi	1,7,1
	subi	7,7,1

	sub	3,3,4
	addi	4,4,-3*16
	subi	5,5,16
	li	8,16

	blt	1,2f

	lfpdux	0,5,8
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	lfpdux	4,5,8
	lfpdux	5,5,8
	lfpdux	6,5,8
	lfpdux	7,5,8
	stfpdux	0,3,4

	beq	1,1f
	mtctr	7

0:	lfpdux	0,5,8
	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	stfpdux	4,3,4
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	lfpdux	4,5,8
	stfpdux	5,3,8
	stfpdux	6,3,8
	stfpdux	7,3,8
	stfpdux	0,3,4
	lfpdux	5,5,8
	lfpdux	6,5,8
	lfpdux	7,5,8
	bdnz	0b

1:	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	stfpdux	4,3,4
	stfpdux	5,3,8
	stfpdux	6,3,8
	stfpdux	7,3,8

2:	beqlr	0

	lfpdux	0,5,8
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	stfpdux	0,3,4
	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8

	blr


.global	_copy_pkt_to_bbuffer_128_bytes
_copy_pkt_to_bbuffer_128_bytes:

	srwi	7,6,1
	andi.	0,6,1
	cmpwi	1,7,1
	subi	7,7,1

	sub	3,3,4
	addi	4,4,-7*16
	subi	5,5,16
	li	8,16

	blt	1,2f

	lfpdux	0,5,8
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	lfpdux	4,5,8
	lfpdux	5,5,8
	lfpdux	6,5,8
	lfpdux	7,5,8
	stfpdux	0,3,4
	lfpdux	8,5,8
	lfpdux	9,5,8
	lfpdux	10,5,8
	lfpdux	11,5,8
	lfpdux	12,5,8
	lfpdux	13,5,8
	lfpdux	14,5,8
	lfpdux	15,5,8

	beq	1,1f
	mtctr	7

	li	9,-16
	stfpdux	14,1,9
	stfpdux	15,1,9

0:	lfpdux	0,5,8
	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	stfpdux	4,3,8
	stfpdux	5,3,8
	stfpdux	6,3,8
	stfpdux	7,3,8
	stfpdux	8,3,4
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	lfpdux	4,5,8
	lfpdux	5,5,8
	lfpdux	6,5,8
	lfpdux	7,5,8
	lfpdux	8,5,8
	stfpdux	9,3,8
	stfpdux	10,3,8
	stfpdux	11,3,8
	stfpdux	12,3,8
	stfpdux	13,3,8
	stfpdux	14,3,8
	stfpdux	15,3,8
	stfpdux	0,3,4
	lfpdux	9,5,8
	lfpdux	10,5,8
	lfpdux	11,5,8
	lfpdux	12,5,8
	lfpdux	13,5,8
	lfpdux	14,5,8
	lfpdux	15,5,8
	bdnz	0b

1:	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	stfpdux	4,3,8
	stfpdux	5,3,8
	stfpdux	6,3,8
	stfpdux	7,3,8
	stfpdux	8,3,4
	stfpdux	9,3,8
	stfpdux	10,3,8
	stfpdux	11,3,8
	stfpdux	12,3,8
	stfpdux	13,3,8
	stfpdux	14,3,8
	stfpdux	15,3,8

	lfpdx	15,0,1
	lfpdux	14,1,8
	addi	1,1,16

2:	beqlr	0

	lfpdux	0,5,8
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	lfpdux	4,5,8
	lfpdux	5,5,8
	lfpdux	6,5,8
	lfpdux	7,5,8
	stfpdux	0,3,4
	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	stfpdux	4,3,8
	stfpdux	5,3,8
	stfpdux	6,3,8
	stfpdux	7,3,8

	blr
#endif
