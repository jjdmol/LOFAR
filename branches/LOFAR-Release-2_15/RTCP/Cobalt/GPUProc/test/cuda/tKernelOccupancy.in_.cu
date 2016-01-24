//# tKernelOccupancy.in_.cu: simple function to test occupancy predictions
//# Copyright (C) 2014  ASTRON (Netherlands Institute for Radio Astronomy)
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

extern "C" {

  struct s {
    float f1, f2, f3, f4, f5, f6, f7, f8;
  };

  __global__ void blkLimit(struct s *buf, unsigned size)
  {
    unsigned i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < size) {
      buf[i].f1 = buf[i].f1 + 1.0f;
    }
  }

  __device__ struct s helper(struct s s, struct s t) {
    s.f1 += s.f8 - s.f5 * t.f1;
    s.f2 *= s.f1 + s.f8 - t.f2;
    s.f3 -= s.f2 * s.f4 + t.f3;
    s.f4 += s.f3 + t.f4 / 2.0f;
    s.f5 *= s.f4 * t.f5 / 5.0f;
    s.f6 -= s.f5 / s.f3 + t.f6 / 11.0f;
    s.f7 += s.f6 + s.f1 * sqrt(t.f7);
    s.f8 *= s.f7 - s.f2 - exp(t.f8);
    return s;
  }

  __global__ void regsLimit(struct s *buf, unsigned size)
  {
    unsigned i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < size) {
      // Try to use quite some reg.
      struct s l, m, n, o, p, q, r;
      do { 
        l = buf[i];
        m = helper(l, l);
        n = helper(m, l);
        o = helper(n, m);
        p = helper(o, n);
        q = helper(p, o);
        r = helper(q, p);
      } while (n.f3 < 0.0f);
      buf[i] = r;
    }
  }

  __global__ void shmemLimit(struct s *buf, unsigned size)
  {
    // Use quite some shmem per block (2 structs * 8 floats * 128 sz * 4 B = 8 kB).
    const unsigned SZ = 128;
    __shared__ struct s s1[SZ];
    __shared__ struct s s2[SZ];

    unsigned i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < size) {
      struct s l = buf[i];
      s1[threadIdx.x] = l;
      struct s m = helper(l, l);
      s2[threadIdx.x] = m;
      __syncthreads();
      l = s1[threadIdx.x + 3 % SZ];
      m = s2[threadIdx.x + 7 % SZ]; 
      buf[i] = helper(m, l);
    }
  }

}

