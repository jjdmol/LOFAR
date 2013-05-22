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

extern "C" {

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


#endif
