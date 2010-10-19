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

	addi	4,4,-1*16
	mtctr	6
	li	8,16
	addi	5,5,-16
	sub	3,3,4

1:	lfpdux	0,5,8
	lfpdux	1,5,8
	stfpdux	0,3,4
	stfpdux	1,3,8
	bdnz	1b
	blr

.global	_copy_pkt_to_bbuffer_64_bytes
_copy_pkt_to_bbuffer_64_bytes:

	addi	4,4,-3*16
	mtctr	6
	li	8,16
	addi	5,5,-16
	sub	3,3,4

1:	lfpdux	0,5,8
	lfpdux	1,5,8
	lfpdux	2,5,8
	lfpdux	3,5,8
	stfpdux	0,3,4
	stfpdux	1,3,8
	stfpdux	2,3,8
	stfpdux	3,3,8
	bdnz	1b
	blr

.global	_copy_pkt_to_bbuffer_128_bytes
_copy_pkt_to_bbuffer_128_bytes:

	addi	4,4,-7*16
	mtctr	6
	li	8,16
	addi	5,5,-16
	sub	3,3,4

1:	lfpdux	0,5,8
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
	bdnz	1b
	blr

#endif
