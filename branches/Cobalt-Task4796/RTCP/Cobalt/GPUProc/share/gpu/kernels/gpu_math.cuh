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
// #define SWIZZLE(ARG, X, Y, Z, W) make_float4((ARG).X, (ARG).Y, (ARG).Z, (ARG).W)
#define SWIZZLE(ARG, X, Y, Z, W) make_double4((ARG).X, (ARG).Y, (ARG).Z, (ARG).W)

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

inline __device__ double4 operator + (double4 a, double4 b)
{
  return make_double4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline __device__ double4 operator - (double4 a, double4 b)
{
  return make_double4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline __device__ double4 operator * (double4 a, double4 b)
{
  return make_double4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline __device__ double4 operator / (double4 a, double4 b)
{
  return make_double4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

inline __device__ double4& operator += (double4 &a, double4 b)
{
  a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w;
  return a;
}

inline __device__ double4& operator -= (double4 &a, double4 b)
{
  a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w;
  return a;
}

inline __device__ double4& operator *= (double4 &a, double4 b)
{
  a.x *= b.x; a.y *= b.y; a.z *= b.z; a.w *= b.w;
  return a;
}

inline __device__ double4& operator /= (double4 &a, double4 b)
{
  a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w;
  return a;
}


#endif
