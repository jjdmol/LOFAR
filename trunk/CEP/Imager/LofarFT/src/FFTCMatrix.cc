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
    ASSERTSTR (size > 0  &&  size%2 == 0,
               "FFTCMatrix size must be positive and even");
    // Only make a new plan when different from previous one.
    // FFTW's plan function is not thread-safe, so guard it.
    if (size != itsSize  ||  forward != itsIsForward) {
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
      scaledFlip (itsSize*itsSize);
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
    flip (data, itsData);
    fftwf_execute (itsPlan);
    flip (itsData, data);
  }

  void FFTCMatrix::backward (uint size, std::complex<float>* data)
  {
    plan (size, false);
    flip (data, itsData);
    fftwf_execute (itsPlan);
    scaledFlip (itsData, data, 1./(size*size));
  }

  void FFTCMatrix::normalized_forward (uint size, std::complex<float>* data)
  {
    plan (size, true);
    flip (data, itsData);
    fftwf_execute (itsPlan);
    scaledFlip (itsData, data, size*size);
  }

  void FFTCMatrix::normalized_backward (uint size, std::complex<float>* data)
  {
    plan (size, false);
    flip (data, itsData);
    fftwf_execute (itsPlan);
    flip (itsData, data);
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

  void FFTCMatrix::flip (const std::complex<float>* in,
                         std::complex<float>* out)
  {
    uint hsz = itsSize/2;
    // Use 2 separate loops to be more cache local.
    // First flip q1 and q4.
    const std::complex<float>* in1  = in;
    const std::complex<float>* in2  = in + hsz*itsSize + hsz;
    std::complex<float>* out1 = out;
    std::complex<float>* out2 = out + hsz*itsSize + hsz;
    for (int k=0; k<2; ++k) {
      for (uint j=0; j<hsz; ++j) {
        for (uint i=0; i<hsz; ++i) {
          *out1++ = *in2++;
          *out2++ = *in1++;
        }
        in1  += hsz;
        in2  += hsz;
        out1 += hsz;
        out2 += hsz;
      }
      // Now flip q2 and q3.
      in1  = in  + hsz;
      in2  = in  + hsz*itsSize;
      out1 = out + hsz;
      out2 = out + hsz*itsSize;
    }
  }

  void FFTCMatrix::scaledFlip (const std::complex<float>* in,
                               std::complex<float>* out,
                               float factor)
  {
    uint hsz = itsSize/2;
    // Use 2 separate loops to be more cache local.
    // First flip q1 and q4.
    const std::complex<float>* in1  = in;
    const std::complex<float>* in2  = in + hsz*itsSize + hsz;
    std::complex<float>* out1 = out;
    std::complex<float>* out2 = out + hsz*itsSize + hsz;
    for (int k=0; k<2; ++k) {
      for (uint j=0; j<hsz; ++j) {
        for (uint i=0; i<hsz; ++i) {
          *out1++ = *in2++ * factor;
          *out2++ = *in1++ * factor;
        }
        in1  += hsz;
        in2  += hsz;
        out1 += hsz;
        out2 += hsz;
      }
      // Now flip q2 and q3.
      in1  = in  + hsz;
      in2  = in  + hsz*itsSize;
      out1 = out + hsz;
      out2 = out + hsz*itsSize;
    }
  }

} //# end namespace

