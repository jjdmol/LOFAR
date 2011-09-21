//# FFTCMatrix.cc: Complex-Complex Fourier transform of a matrix
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include <LofarFT/FFTCMatrix.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/ArrayMath.h>

namespace LOFAR {

  FFTCMatrix::FFTCMatrix()
    : itsData      (0),
      itsPlan      (0),
      itsSize      (0),
      itsReserved  (0),
      itsIsForward (false)
  {}

  FFTCMatrix::FFTCMatrix (const FFTCMatrix& that)
    : itsData      (0),
      itsPlan      (0),
      itsSize      (0),
      itsReserved  (0),
      itsIsForward (false)
  {
    reserve (that.itsReserved);
  }

  FFTCMatrix& FFTCMatrix::operator= (const FFTCMatrix& that)
  {
    if (this != &that) {
      reserve (that.itsReserved);
    }
    return *this;
  }

  void FFTCMatrix::clear()
  {
    if (itsData) {
      fftw_free (itsData);
    }
    if (itsPlan) {
      fftwf_destroy_plan (itsPlan);
    }
    itsData     = 0;
    itsPlan     = 0;
    itsSize     = 0;
    itsReserved = 0;
  }

  void FFTCMatrix::reserve (uint size)
  {
    if (size != itsReserved) {
      clear();
      if (size > 0) {
        itsData = static_cast<std::complex<float>*>
                  (fftw_malloc ((size*size)*sizeof(std::complex<float>)));
        itsReserved = size;
      }
    }
  }

  void FFTCMatrix::plan (uint size, bool forward)
  {
    ASSERTSTR (size > 0, "FFTCMatrix size must be positive");
    // Only make a new plan when different from previous one.
    // FFTW's plan function is not thread-safe, so guard it.
    if (itsPlan == 0  ||  size != itsSize  ||  forward != itsIsForward) {
#pragma omp critical(fftcmatrix_plan)
      {
        if (size > itsReserved) {
          reserve (size);
        }
        itsSize = size;
        itsIsForward = forward;
        int direction = (forward  ?  FFTW_FORWARD : FFTW_BACKWARD);
        if (itsPlan) {
          fftwf_destroy_plan (itsPlan);
          itsPlan = 0;
        }
        itsPlan = fftwf_plan_dft_2d(itsSize, itsSize,
                                    reinterpret_cast<fftwf_complex*>(itsData),
                                    reinterpret_cast<fftwf_complex*>(itsData),
                                    direction, FFTW_ESTIMATE);
      }
    }
  }

  void FFTCMatrix::fft()
  {
    if (itsIsForward) {
      if (itsSize%4 == 0) {
        negatedFlip();
        fftwf_execute (itsPlan);
      } else {
        flip();
        fftwf_execute (itsPlan);
        flip();
      }
    } else {
      flip();
      fftwf_execute (itsPlan);
      scaledFlip (1. / (itsSize*itsSize));
    }
  }

  void FFTCMatrix::normalized_fft()
  {
    if (itsIsForward) {
      flip();
      fftwf_execute (itsPlan);
      scaledFlip (1. / (itsSize*itsSize));
    } else {
      if (itsSize%4 == 0) {
        negatedFlip();
        fftwf_execute (itsPlan);
      } else {
        flip();
        fftwf_execute (itsPlan);
        flip();
      }
    }
  }

  void FFTCMatrix::forward (uint size, std::complex<float>* data)
  {
    plan (size, true);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    flip (itsData, data, false);
  }

  void FFTCMatrix::backward (uint size, std::complex<float>* data)
  {
    plan (size, false);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    scaledFlip (itsData, data, false, 1./(size*size));
  }

  void FFTCMatrix::normalized_forward (uint size, std::complex<float>* data)
  {
    plan (size, true);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    scaledFlip (itsData, data, false, 1./(size*size));
  }

  void FFTCMatrix::normalized_backward (uint size, std::complex<float>* data)
  {
    plan (size, false);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    flip (itsData, data, false);
  }

  // Flip the quadrants which is needed for the FFT.
  //  q1 q2    gets   q4 q3
  //  q3 q4           q2 q1
  void FFTCMatrix::flip()
  {
    uint hsz = itsSize/2;
    // Use 2 separate loops to be more cache local.
    // First flip q1 and q4.
    std::complex<float>* p1 = itsData;
    std::complex<float>* p2 = itsData + hsz*itsSize + hsz;
    for (int k=0; k<2; ++k) {
      for (uint j=0; j<hsz; ++j) {
        for (uint i=0; i<hsz; ++i) {
          std::complex<float> tmp1 = *p1;
          *p1++ = *p2;
          *p2++ = tmp1;
        }
        p1 += hsz;
        p2 += hsz;
      }
      // Now flip q2 and q3.
      p1 = itsData + hsz;
      p2 = itsData + hsz*itsSize;
    }
  }

  // The output flip can be avoided by negating every other input element.
  // So do the flip and negation jointly (only for multiple of 4 elements).
  void  FFTCMatrix::negatedFlip()
  {
    DBGASSERT (itsSize%4==0);
    uint hsz = itsSize/2;
    uint hhsz = hsz/2;
    // Even elements do not need to be negated.
    // First handle even lines.
    std::complex<float>* p1 = itsData;
    std::complex<float>* p2 = itsData + hsz*itsSize + hsz;
    for (int k=0; k<2; ++k) {
      for (uint j=0; j<hhsz; ++j) {
        // Handle even lines.
        for (uint i=0; i<hhsz; ++i) {
          std::complex<float> tmp1 = *p1;
          *p1++ = *p2;
          *p2++ = tmp1;
          std::complex<float> tmp2 = -*p1;
          *p1++ = -*p2;
          *p2++ = tmp2;
        }
        p1 += hsz;
        p2 += hsz;
        // Handle odd lines.
        for (uint i=0; i<hhsz; ++i) {
          std::complex<float> tmp1 = -*p1;
          *p1++ = -*p2;
          *p2++ = tmp1;
          std::complex<float> tmp2 = *p1;
          *p1++ = *p2;
          *p2++ = tmp2;
        }
        p1 += hsz;
        p2 += hsz;
      }
      // Now flip q2 and q3.
      p1 = itsData + hsz;
      p2 = itsData + hsz*itsSize;
    }
  }

  void FFTCMatrix::scaledFlip (float factor)
  {
    uint hsz = itsSize/2;
    // Use 2 separate loops to be more cache local.
    // First flip q1 and q4.
    std::complex<float>* p1 = itsData;
    std::complex<float>* p2 = itsData + hsz*itsSize + hsz;
    for (int k=0; k<2; ++k) {
      for (uint j=0; j<hsz; ++j) {
        for (uint i=0; i<hsz; ++i) {
          std::complex<float> tmp1 = *p1 * factor;
          *p1++ = *p2 * factor;
          *p2++ = tmp1;
        }
        p1 += hsz;
        p2 += hsz;
      }
      // Now flip q2 and q3.
      p1 = itsData + hsz;
      p2 = itsData + hsz*itsSize;
    }
  }

  void FFTCMatrix::flip (const std::complex<float>* __restrict__ in,
                         std::complex<float>* __restrict__ out,
                         bool toZero)
  {
    uint hsz0 = itsSize/2;
    uint hsz1 = hsz0;
    if (2*hsz0 != itsSize) {
      if (toZero) {
        hsz1++;
      } else {
        hsz0++;
      }
    }
    // Use 4 separate loops to move the quadrants.
    // q1
    const std::complex<float>* __restrict__ fr = in;
    std::complex<float>* __restrict__ to = out + hsz1*itsSize + hsz1;
    for (uint j=0; j<hsz0; ++j) {
      for (uint i=0; i<hsz0; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
    // q2
    fr = in + hsz0;
    to = out + hsz1*itsSize;
    for (uint j=0; j<hsz0; ++j) {
      for (uint i=0; i<hsz1; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
    // q3
    fr = in + hsz0*itsSize;
    to = out + hsz1;
    for (uint j=0; j<hsz1; ++j) {
      for (uint i=0; i<hsz0; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
    // q4
    fr = in + hsz0*itsSize + hsz0;
    to = out;
    for (uint j=0; j<hsz1; ++j) {
      for (uint i=0; i<hsz1; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
  }

  void FFTCMatrix::scaledFlip (const std::complex<float>* __restrict__ in,
                               std::complex<float>* __restrict__ out,
                               bool toZero,
                               float factor)
  {
    uint hsz0 = itsSize/2;
    uint hsz1 = hsz0;
    if (2*hsz0 != itsSize) {
      if (toZero) {
        hsz1++;
      } else {
        hsz0++;
      }
    }
    // Use 4 separate loops to move the quadrants.
    // q1
    const std::complex<float>* __restrict__ fr = in;
    std::complex<float>* __restrict__ to = out + hsz1*itsSize + hsz1;
    for (uint j=0; j<hsz0; ++j) {
      for (uint i=0; i<hsz0; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
    // q2
    fr = in + hsz0;
    to = out + hsz1*itsSize;
    for (uint j=0; j<hsz0; ++j) {
      for (uint i=0; i<hsz1; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
    // q3
    fr = in + hsz0*itsSize;
    to = out + hsz1;
    for (uint j=0; j<hsz1; ++j) {
      for (uint i=0; i<hsz0; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
    // q4
    fr = in + hsz0*itsSize + hsz0;
    to = out;
    for (uint j=0; j<hsz1; ++j) {
      for (uint i=0; i<hsz1; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
  }

} //# end namespace

