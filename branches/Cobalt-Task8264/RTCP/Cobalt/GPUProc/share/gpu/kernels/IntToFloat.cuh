//# IntToFloat.cuh: Helper function for converting integers to float
//#
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_GPUPROC_CUDA_INTTOFLOAT_CUH
#define LOFAR_GPUPROC_CUDA_INTTOFLOAT_CUH

// \file
// Device function to convert integer types to float.
// This file contains a helper function for converting integer types to floats.
// The actual function used depends on the define \c NR_BITS_PER_SAMPLE
// If this is 8 the input char get convert with instances of -128 clamped
// to -127.
// If <tt>NR_BITS_PER_SAMPLE == 16</tt> a simple conversion to float is performed.
//
// Output values are scaled in terms of 16 bit mode.

#if NR_BITS_PER_SAMPLE == 16
inline __device__ float convertIntToFloat(short x)
{
	return x;
}
#elif NR_BITS_PER_SAMPLE == 8
inline __device__ float convertIntToFloat(signed char x)
{
  // Edge case. -128 should be returned as -127
  int i = x == -128 ? -127 : x;

  // Keep output scale the same as 16 bit mode.
  // Gains (input and complex voltages) end up x16,
  // power (visibilities and Stokes) end up x16^2.
  return 16 * i;
}
#elif NR_BITS_PER_SAMPLE == 4
// Extract the 4-bit real or imaginary part of an 8-bit input sample
inline __device__ signed char extractRI(signed char x, bool imag)
{
  // Note 1: Imaginary part is in the top 4 bits. See also RSP::decode4bit() in InputProc/Station/RSP.h.
  // Note 2: Preserve the sign, so use sign-extending shifts to extract the right bits.
  return imag ? x >> 4 : (x << 4) >> 4;
}

// WARNING: Caller is responsible for extracting 4-bit real or imaginary part from sample byte
inline __device__ float convertIntToFloat(signed char x)
{
  // Edge case. -8 should be returned as -7
  int i = x == -8 ? -7 : x;

  // TODO: Is this the right scaling for 4-bit mode?
  // Keep output scale the same as 16 bit mode.
  return 64 * i;
}
#else
#error unsupported NR_BITS_PER_SAMPLE
#endif

#endif

