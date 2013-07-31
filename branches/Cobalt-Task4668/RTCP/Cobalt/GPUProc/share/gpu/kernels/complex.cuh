//# complex.cuh: complex type for CUDA device code (limited)
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

#ifndef LOFAR_GPUPROC_CUDA_COMPLEX_CUH
#define LOFAR_GPUPROC_CUDA_COMPLEX_CUH

// Simple 'complex type' template for CUDA device code.
// See remarks below for the (intentional) limitations.
template <typename T>
struct complex {
  // preferably private, but we need its address for sincos()
  float2 m_z; // aligned (m_z.x, m_z.y)

  typedef T value_type;

  __device__ complex() {
    // Leave m_z.x and m_z.y uninit. We don't need it in GPU code.
    // Apart from the overhead in GPU code, we have already been hit by an
    // init bug wrt thread races on shmem. (nvcc even warns in some cases.)
  }

  __device__ complex(const T& real) {
    m_z.x = real;
    // Leave m_z.y uninit. Avoid T().
  }

  __device__ complex(const T& real, const T& imag) {
    m_z.x = real;
    m_z.y = imag;
  }

  __device__ complex(const complex<T>& val) {
    m_z.x = val.real();
    m_z.y = val.imag();
  }

  //template <typename U>
  __device__ complex<T>& operator=(const complex</*U*/T>& rhs) {
    m_z.x = rhs.real();
    m_z.y = rhs.imag();
    return *this;
  }

  __device__ T real() const { return m_z.x; }
  __device__ T imag() const { return m_z.y; }
  __device__ void real(T value) { m_z.x = value; }
  __device__ void imag(T value) { m_z.y = value; }


  // We need these, because of the missing constructor inits.
  // Otherwise, the constructor could convert T& to complex<T>& and
  // use the operators after these, but they read rhs.imag().
  __device__ complex<T>& operator+=(const T& real) {
    m_z.x += real;
    return *this;
  }

  __device__ complex<T>& operator-=(const T& real) {
    m_z.x -= real;
    return *this;
  }

  __device__ complex<T>& operator*=(const T& real) {
    m_z.x *= real;
    m_z.y *= real;
    return *this;
  }

  // Don't bother supporting operator/= and /: we don't need them.


  // Don't bother supporting the cross-type variants. Only need float.
  //template <typename U>
  __device__ complex<T>& operator+=(complex</*U*/T>& rhs) {
    m_z.x += rhs.real();
    m_z.y += rhs.imag();
    return *this;
  }

  //template <typename U>
  __device__ complex<T>& operator-=(complex</*U*/T>& rhs) {
    m_z.x -= rhs.real();
    m_z.y -= rhs.imag();
    return *this;
  }

  //template <typename U>
  __device__ complex<T>& operator*=(complex</*U*/T>& rhs) {
    m_z.x = m_z.x * rhs.real() - m_z.y * rhs.imag();
    m_z.y = m_z.x * rhs.imag() + m_z.y * rhs.real();
    return *this;
  }
};


template <typename T>
__device__ complex<T> operator+(const complex<T>& rhs) {
  return rhs;
}

template <typename T>
__device__ complex<T> operator-(const complex<T>& rhs) {
  return complex<T>(-rhs.real(), -rhs.imag());
}


template <typename T>
__device__ complex<T> operator+(const T& lhs, const complex<T>& rhs) {
  return complex<T>(lhs + rhs.real(), rhs.imag());
}

template <typename T>
__device__ complex<T> operator+(const complex<T>& lhs, const T& rhs) {
  return complex<T>(lhs.real() + rhs, lhs.imag());
}

template <typename T>
__device__ complex<T> operator+(const complex<T>& lhs, const complex<T>& rhs) {
  return complex<T>(lhs.real() + rhs.real(), lhs.imag() + rhs.imag());
}

template <typename T>
__device__ complex<T> operator-(const T& lhs, const complex<T>& rhs) {
  return complex<T>(lhs - rhs.real(), rhs.imag());
}

template <typename T>
__device__ complex<T> operator-(const complex<T>& lhs, const T& rhs) {
  return complex<T>(lhs.real() - rhs, lhs.imag());
}

template <typename T>
__device__ complex<T> operator-(const complex<T>& lhs, const complex<T>& rhs) {
  return complex<T>(lhs.real() - rhs.real(), lhs.imag() - rhs.imag());
}


template <typename T>
__device__ complex<T> operator*(const T& lhs, const complex<T>& rhs) {
  return complex<T>(lhs * rhs.real(), lhs * rhs.imag());
}

template <typename T>
__device__ complex<T> operator*(const complex<T>& lhs, const T& rhs) {
  return complex<T>(lhs.real() * rhs, lhs.imag() * rhs);
}

template <typename T>
__device__ complex<T> operator*(const complex<T>& lhs, const complex<T>& rhs) {
  return complex<T>(lhs.real() * rhs.real() - lhs.imag() * rhs.imag(),
                    lhs.real() * rhs.imag() + lhs.imag() * rhs.real());
}


#endif

