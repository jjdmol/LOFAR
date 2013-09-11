//# gpu_math.cuh: Functions and operators for CUDA-specific types.
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

#ifndef LOFAR_GPUPROC_CUDA_GPU_MATH_CUH
#define LOFAR_GPUPROC_CUDA_GPU_MATH_CUH

// \file cuda/gpu_math.cuh
// Functions and operators for CUDA-specific types.
// This file contains functions and operators for CUDA-specific types, like
// float4. Only a minimal set of operators is provided, the ones that are
// currently needed. It can be extended when needed. We do \e not plan to
// provide a complete set of C++-operators for all the different CUDA types.

#include <vector_functions.h>

// Provide the equivalent of the OpenCL swizzle feature. Obviously, the OpenCL
// \c swizzle operation is more powerful, because it's a language built-in.
#define SWIZZLE(ARG, X, Y, Z, W) make_float4((ARG).X, (ARG).Y, (ARG).Z, (ARG).W)

inline __device__ float4 operator + (float4 a, float4 b)
{
  return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline __device__ float4 operator - (float4 a, float4 b)
{
  return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline __device__ float4 operator * (float4 a, float4 b)
{
  return make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline __device__ float4 operator / (float4 a, float4 b)
{
  return make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

inline __device__ float4& operator += (float4 &a, float4 b)
{
  a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w;
  return a;
}

inline __device__ float4& operator -= (float4 &a, float4 b)
{
  a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w;
  return a;
}

inline __device__ float4& operator *= (float4 &a, float4 b)
{
  a.x *= b.x; a.y *= b.y; a.z *= b.z; a.w *= b.w;
  return a;
}

inline __device__ float4& operator /= (float4 &a, float4 b)
{
  a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w;
  return a;
}

inline __device__ float2 operator * (float2 a, float b)
{
  return make_float2(a.x * b, a.y);
}

inline __device__ float2 operator * (float a, float2 b)
{
  return make_float2(b.x * a, b.y);
}



// to distinguish complex float/double from other uses of float2/double2
typedef float2 fcomplex;
typedef double2 dcomplex;

typedef char2  char_complex;
typedef short2 short_complex;

// Keep it simple. We had complex<T> defined, but we need operator overloads,
// so we cannot make it a POD type. Then we got redundant member inits in the
// constructor, causing races when declaring variables in shared memory.
// Now, avoid complex<T> and just work with cmul() and a few extra lines.
__device__ fcomplex cmul(fcomplex lhs, fcomplex rhs)
{
  return make_float2(lhs.x * rhs.x - lhs.y * rhs.y,
                     lhs.x * rhs.y + lhs.y * rhs.x);
}

__device__ fcomplex phaseShift(float frequency, float delay)
{
  // Convert the fraction of sample duration (delayAtBegin/delayAfterEnd) to fractions of a circle.
  // Because we `undo' the delay, we need to rotate BACK.
  const float pi2 = -6.28318530717958647688f; // -2.0f * M_PI_F
  float phaseShift = delay * frequency;
  float phi = pi2 * phaseShift;

  fcomplex rv;
  sincosf(phi, &rv.y, &rv.x); // store (cos(), sin())
  return rv;
}

#endif

