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

#include <lofar_config.h>


#include <AWImager2/FFTCMatrix.h>
#include <Common/LofarLogger.h>
#include <casacore/casa/Utilities/Copy.h>
#include <casacore/casa/OS/Path.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <cstring>

using namespace casa;

namespace LOFAR {

  bool FFTCMatrix::theirInitDone = false;

  FFTCMatrix::FFTCMatrix()
    : itsData      (0),
      itsPlan      (0),
      itsSize      (0),
      itsReserved  (0),
      itsPaddingBuffer0(0),
      itsPaddingBuffer1(0),
      itsPaddingBuffer2(0),
      itsPaddingBuffer3(0),
      itsPaddingPlan0(0),
      itsPaddingPlan1(0),
      itsPaddingInputSize(0),
      itsPadding(0),
      itsNThreads  (0),
      itsIsForward (false)
  {
    // If the first time, set to multi-threading and
    // read the wisdom from the system file.
    if (!theirInitDone) {
#pragma omp critical(fftcmatrix_init)
      {
        if (!theirInitDone) {
          fftwf_init_threads();
          FILE* file = fopen (casa::Path("$HOME/fftwisdom2d.txt").
                              expandedName().c_str(), "r");
          if (!file) {
            file = fopen (casa::Path("$LOFARLOCALROOT/fftwisdom2d.txt").
                          expandedName().c_str(), "r");
          }
          if (!file) {
            file = fopen (casa::Path("$LOFARROOT/fftwisdom2d.txt").
                          expandedName().c_str(), "r");
          }
          if (!file) {
            file = fopen ("/opt/lofar/fftwisdom2d.txt", "r");
          }
          if (!file) {
            file = fopen ("/etc/fftw/fftwisdom2d.txt", "r");
          }
          if (file) {
            fftw_import_wisdom_from_file (file);
            fclose (file);
          }
          theirInitDone = true;
        }
      } // end omp critical
    }
  }

  FFTCMatrix::FFTCMatrix (const FFTCMatrix& that)
    : itsData      (0),
      itsPlan      (0),
      itsSize      (0),
      itsReserved  (0),
      itsPaddingBuffer0(0),
      itsPaddingBuffer1(0),
      itsPaddingBuffer2(0),
      itsPaddingBuffer3(0),
      itsPaddingPlan0(0),
      itsPaddingPlan1(0),
      itsPaddingInputSize(0),
      itsPadding(0),
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
    if (itsData) fftw_free (itsData);
    if (itsPlan) fftwf_destroy_plan (itsPlan);

    itsData     = 0;
    itsPlan     = 0;
    itsSize     = 0;
    itsReserved = 0;
  }

  void FFTCMatrix::clear_padding()
  {
    if (itsPaddingPlan0) fftwf_destroy_plan (itsPaddingPlan0);
    if (itsPaddingPlan1) fftwf_destroy_plan (itsPaddingPlan1);
    if (itsPaddingBuffer0) fftwf_free(itsPaddingBuffer0);
    if (itsPaddingBuffer1) fftwf_free(itsPaddingBuffer1);
    if (itsPaddingBuffer2) fftwf_free(itsPaddingBuffer2);
    if (itsPaddingBuffer3) fftwf_free(itsPaddingBuffer3);

    itsPaddingPlan0 = 0;
    itsPaddingPlan1 = 0;
    itsPaddingBuffer0 = 0;
    itsPaddingBuffer1 = 0;
    itsPaddingBuffer2 = 0;
    itsPaddingBuffer3 = 0;
    itsPaddingInputSize = 0;
    itsPadding = 0;
  }

  void FFTCMatrix::reserve (size_t size)
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

  void FFTCMatrix::plan (size_t size, bool forward, int nthreads,
                         unsigned flags)
  {
    ASSERTSTR (size > 0, "FFTCMatrix size must be positive");
    // Only make a new plan when different from previous one.
    // FFTW's plan function is not thread-safe, so guard it.
    if (itsPlan == 0  ||  size != itsSize  ||
        nthreads != itsNThreads  ||  forward != itsIsForward) {
#pragma omp critical(fftcmatrix_plan)
      {
        if (size > itsReserved) {
          reserve (size);
        }
        itsSize = size;
        itsNThreads = nthreads;
        itsIsForward = forward;
        int direction = (forward  ?  FFTW_FORWARD : FFTW_BACKWARD);
        if (itsPlan) {
          fftwf_destroy_plan (itsPlan);
          itsPlan = 0;
        }
        fftwf_plan_with_nthreads (nthreads);
        itsPlan = fftwf_plan_dft_2d(itsSize, itsSize,
                                    reinterpret_cast<fftwf_complex*>(itsData),
                                    reinterpret_cast<fftwf_complex*>(itsData),
                                    direction, flags);
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
        flip (true);
        fftwf_execute (itsPlan);
        flip (false);
      }
    } else {
      flip (true);
      fftwf_execute (itsPlan);
      scaledFlip (false, 1./(itsSize*itsSize));
    }
  }

  void FFTCMatrix::normalized_fft()
  {
    if (itsIsForward) 
    {
      flip (true);
      fftwf_execute (itsPlan);
      scaledFlip (false, 1./(itsSize*itsSize));
    } 
    else 
    {
      if (itsSize%4 == 0) 
      {
        negatedFlip();
        fftwf_execute (itsPlan);
      } 
      else 
      {
        flip (true);
        fftwf_execute (itsPlan);
        flip (false);
      }
    }
  }

  void FFTCMatrix::forward (size_t size, std::complex<float>* data,
                            int nthreads,
			    unsigned flags)
  {
    plan (size, true, nthreads, flags);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    flip (itsData, data, false);
  }

  void FFTCMatrix::backward (size_t size, std::complex<float>* data,
                             int nthreads,
			     unsigned flags)
  {
    plan (size, false, nthreads, flags);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    scaledFlip (itsData, data, false, 1./(size*size));
  }

  void FFTCMatrix::normalized_forward (size_t size, std::complex<float>* data,
                                       int nthreads,
				       unsigned flags)
  {
    plan (size, true, nthreads, flags);
    flip (data, itsData, true);
    fftwf_execute (itsPlan);
    scaledFlip (itsData, data, false, 1./(size*size));
  }

  void FFTCMatrix::normalized_backward (size_t size, std::complex<float>* data,
                                        int nthreads,
                                        unsigned flags)
    {
      plan (size, false, nthreads, flags);
      flip (data, itsData, true);
      fftwf_execute (itsPlan);
      flip (itsData, data, false);
    }
    
  void FFTCMatrix::init_padding(size_t n, int padding)
  {
    ASSERTSTR ((n % 2) == 1, "FFTCMatrix padding input size must be odd");
    ASSERTSTR ((padding % 2) == 1, "FFTCMatrix padding factor must be odd");
  
    clear_padding();
    
    itsPaddingInputSize = n;
    itsPadding = padding;

    // fftw allocate buffer 0 
//     itsPaddingBuffer0 = fftwf_alloc_complex(n * n * padding);
    itsPaddingBuffer0 = (fftwf_complex *)fftwf_malloc(n * n * padding *sizeof(fftwf_complex));
    // fill buffer with zeros (zero padding)
    memset( itsPaddingBuffer0, 0, sizeof ( fftwf_complex ) * n * n * padding);
    
    // fftw allocate buffer 1
//     itsPaddingBuffer1 = fftwf_alloc_complex(n * n * padding);
    itsPaddingBuffer1 = (fftwf_complex *)fftwf_malloc(n * n * padding *sizeof(fftwf_complex));

    // fftw allocate buffer 2
//     itsPaddingBuffer2 = fftwf_alloc_complex(n * n * padding * padding);
    itsPaddingBuffer2 = (fftwf_complex *)fftwf_malloc(n * n * padding * padding *sizeof(fftwf_complex));
    memset( itsPaddingBuffer2, 0, sizeof ( fftwf_complex ) * n * n * padding * padding);

    // fftw allocate buffer 3
//     itsPaddingBuffer3 = fftwf_alloc_complex(n * n * padding * padding);
    itsPaddingBuffer3 = (fftwf_complex *)fftwf_malloc(n * n * padding * padding *sizeof(fftwf_complex));
    
    // plan many fft
    
    int rank = 1;
    int n1[] = {n*padding};
    int howmany = n;
    int idist = n*padding;
    int odist = n*padding;
    int istride = 1;
    int ostride = 1;
    int *inembed = NULL;
    int *onembed = NULL;
    
    #pragma omp critical(fftcmatrix_plan)
    {
      itsPaddingPlan0 = fftwf_plan_many_dft(rank, n1, howmany,
                                    itsPaddingBuffer0, inembed,istride, idist,
                                    itsPaddingBuffer1, onembed, ostride, odist,
                                    FFTW_FORWARD, FFTW_MEASURE);

      howmany = n*padding;    
      itsPaddingPlan1 = fftwf_plan_many_dft(rank, n1, howmany,
                          itsPaddingBuffer2, inembed,istride, idist,
                          itsPaddingBuffer3, onembed, ostride, odist,
                          FFTW_FORWARD, FFTW_MEASURE);
    }
    
  }

  
  Matrix<Complex> FFTCMatrix::padded_forward (Matrix<Complex> data, 
                            int padding,
                            int nthreads,
                            unsigned flags)
  {
    size_t nx = data.shape()(0);
    size_t ny = data.shape()(1);
    ASSERTSTR (nx == ny, "FFTCMatrix matrix must be square");
    size_t n = nx;
    ASSERTSTR (itsPaddingInputSize == n, "FFTCMatrix padding matrix size mismatch init");
    ASSERTSTR (itsPadding == padding, "FFTCMatrix padding mismatch init");
    

    // flip transpose
    size_t center_in = n/2;
    size_t center_out = (n*padding)/2;
    
    // Use 2 separate loops to move the halves

    // upper halve
    const std::complex<float>* __restrict__ fr = (data.data());
    std::complex<float>* __restrict__ to = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer0) + n*padding-center_in;
    for (size_t j=0; j<center_in; ++j) {
      for (size_t i=0; i<n; ++i) 
      {
        to[j+i*n*padding] = fr[j*n+i];
      }
    }
    
    // lower halve
    fr = data.data() + center_in*n;
    to = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer0);
    for (size_t j=0; j<=center_in; ++j) {
      for (size_t i=0; i<n; ++i) {
        to[j+i*n*padding] = fr[j*n+i];
      }
    }

    fftwf_execute ( itsPaddingPlan0 );
    
    // transpose zeropad

    // upper halve
    fr = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer1);
    to = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer2) + n*padding-center_in;
    for (size_t j=0; j<center_in; ++j) {
      for (size_t i=0; i<n*padding; ++i) {
        to[j+i*n*padding] = fr[j*n*padding+i];
      }
    }
    
    // lower halve
    fr = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer1) + center_in*n*padding;
    to = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer2);
    for (size_t j=0; j<=center_in; ++j) {
      for (size_t i=0; i<n*padding; ++i) {
        to[j+i*n*padding] = fr[j*n*padding+i];
      }
    }
    
    fftwf_execute ( itsPaddingPlan1 );
    
    Matrix<Complex> result(IPosition(2, n*padding, n*padding));
    
    size_t n_out = n*padding;
    
    // Use 4 separate loops to move the quadrants.
    // q1
    fr = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer3);
    to = result.data();
    for (size_t j=0; j<=center_out; ++j) 
    {
      for (size_t i=0; i<=center_out; ++i) 
      {
        Int ii = i+center_out;
        Int jj = j+center_out;
        Int ii0 = (ii % padding);
        Int ii1 = (ii / padding);
        Int jj0 = (jj % padding);
        Int jj1 = (jj / padding);
        to[ (ii0 + jj0*padding)*n*n + ii1 + jj1*n ] = fr[i+j*n_out]/int(n*n);
      }
    }
    // q2
    fr = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer3) + center_out+1;
    for (size_t j=0; j<=center_out; ++j) {
      for (size_t i=0; i<center_out; ++i) 
      {
        Int ii = i;
        Int jj = j + center_out;
        Int ii0 = (ii % padding);
        Int ii1 = (ii / padding);
        Int jj0 = (jj % padding);
        Int jj1 = (jj / padding);
        to[ (ii0 + jj0*padding)*n*n + ii1 + jj1*n ] = fr[i+j*n_out]/int(n*n);
      }
    }
    // q3
    fr = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer3) + (center_out+1)*n_out;
    for (size_t j=0; j<center_out; ++j) {
      for (size_t i=0; i<=center_out; ++i) 
      {
        Int ii = i + center_out;
        Int jj = j;
        Int ii0 = (ii % padding);
        Int ii1 = (ii / padding);
        Int jj0 = (jj % padding);
        Int jj1 = (jj / padding);
        to[ (ii0 + jj0*padding)*n*n + ii1 + jj1*n ] = fr[i+j*n_out]/int(n*n);
      }
    }
    // q4
    fr = reinterpret_cast<std::complex<float>*>(itsPaddingBuffer3) + (center_out+1)*n_out+center_out+1;
    for (size_t j=0; j<center_out; ++j) {
      for (size_t i=0; i<center_out; ++i) 
      {
        Int ii = i;
        Int jj = j;
        Int ii0 = (ii % padding);
        Int ii1 = (ii / padding);
        Int jj0 = (jj % padding);
        Int jj1 = (jj / padding);
        to[ (ii0 + jj0*padding)*n*n + ii1 + jj1*n ] = fr[i+j*n_out]/int(n*n);
      }
    }
    
    return result;
  }

  Matrix<Complex> FFTCMatrix::padded_backward (Matrix<Complex> data, 
                            int padding,
                            int nthreads,
                            unsigned flags)
  {
    
  }


  // Flip the quadrants which is needed for the FFT.
  //  q1 q2    gets   q4 q3
  //  q3 q4           q2 q1
  void FFTCMatrix::flip (bool toZero)
  {
    size_t hsz = itsSize/2;
    if (2*hsz != itsSize) {
      flipOdd (toZero);
    } else {
      // Use 2 separate loops to be more cache local.
      // First flip q1 and q4.
      std::complex<float>* p1 = itsData;
      std::complex<float>* p2 = itsData + hsz*itsSize + hsz;
      for (int k=0; k<2; ++k) {
        for (size_t j=0; j<hsz; ++j) {
          for (size_t i=0; i<hsz; ++i) {
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
  }

  // The output flip can be avoided by negating every other input element.
  // So do the flip and negation jointly (only for multiple of 4 elements).
  void  FFTCMatrix::negatedFlip()
  {
    DBGASSERT (itsSize%4==0);
    size_t hsz = itsSize/2;
    size_t hhsz = hsz/2;
    // Even elements do not need to be negated.
    // First handle even lines.
    std::complex<float>* p1 = itsData;
    std::complex<float>* p2 = itsData + hsz*itsSize + hsz;
    for (int k=0; k<2; ++k) {
      for (size_t j=0; j<hhsz; ++j) {
        // Handle even lines.
        for (size_t i=0; i<hhsz; ++i) {
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
        for (size_t i=0; i<hhsz; ++i) {
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

  void FFTCMatrix::scaledFlip (bool toZero, float factor)
  {
    size_t hsz = itsSize/2;
    if (2*hsz != itsSize) {
      // It would be faster to make a flipScaledOdd, but this will do.
      flipOdd (toZero);
      for (size_t i=0; i<itsSize*itsSize; ++i) {
        itsData[i] *= factor;
      }
    } else {
      // Use 2 separate loops to be more cache local.
      // First flip q1 and q4.
      std::complex<float>* p1 = itsData;
      std::complex<float>* p2 = itsData + hsz*itsSize + hsz;
      for (int k=0; k<2; ++k) {
        for (size_t j=0; j<hsz; ++j) {
          for (size_t i=0; i<hsz; ++i) {
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
  }

  void FFTCMatrix::flipOdd (bool toZero)
  {
    int hsz = itsSize/2;
    int lhsz = hsz;
    int rhsz = hsz;
    // Save the middle row and column.
    std::vector<std::complex<float> > tmprow(itsSize);
    std::vector<std::complex<float> > tmpcol(itsSize);
    std::complex<float>* tmprowPtr = &(tmprow[0]);
    std::complex<float>* tmpcolPtr = &(tmpcol[0]);
    casa::objcopy (tmprowPtr, itsData + hsz*itsSize, itsSize);
    casa::objcopy (tmpcolPtr, itsData + hsz, itsSize, 1, itsSize);
    std::complex<float>* __restrict__ p1f;
    std::complex<float>* __restrict__ p1t;
    std::complex<float>* __restrict__ p2f;
    std::complex<float>* __restrict__ p2t;
    int incr = itsSize;
    int outm = itsSize-1;
    // Determine where to start moving elements around.
    // Move to the middle line first, because that one is saved.
    if (toZero) {
      p1f = itsData + itsSize*itsSize - hsz;
      p1t = itsData + hsz*itsSize + 1;
      p2f = itsData + hsz*itsSize - itsSize;
      p2t = p1f;
      incr = -itsSize;
      outm = 0;
      rhsz++;
    } else {
      p1f = itsData;
      p1t = itsData + hsz*itsSize + hsz;
      p2f = p1t + itsSize + 1;
      p2t = itsData;
      lhsz++;
    }
    // Exchange q1 and q4.
    // Use separate loops for p1t and p2t, otherwise overwrites can occur.
    for (int j=0; j<hsz; ++j) {
      for (int i=0; i<hsz; ++i) {
        p1t[i] = p1f[i];
      }
      for (int i=0; i<hsz; ++i) {
        p2t[i] = p2f[i];
      }
      p1f += incr;
      p1t += incr;
      p2f += incr;
      p2t += incr;
    }
    if (toZero) {
      p1f = itsData + itsSize*itsSize - itsSize;
      p1t = itsData + hsz*itsSize - hsz + itsSize;
      p2f = itsData + hsz*itsSize - hsz;
      p2t = itsData + itsSize*itsSize - itsSize + 1;
    } else {
      p1f = itsData + hsz + 1;
      p1t = itsData + hsz*itsSize;
      p2f = p1t + itsSize;
      p2t = itsData + hsz;
    }
    // Exchange q2 and a3.
    for (int j=0; j<hsz; ++j) {
      for (int i=0; i<hsz; ++i) {
        p1t[i] = p1f[i];
      }
      for (int i=0; i<hsz; ++i) {
        p2t[i] = p2f[i];
      }
      p1f += incr;
      p1t += incr;
      p2f += incr;
      p2t += incr;
    }
    // Put back the middle row and column while exchanging top and bottom.
    casa::objcopy (itsData + outm*itsSize + rhsz, tmprowPtr, lhsz);
    casa::objcopy (itsData + outm*itsSize, tmprowPtr + lhsz, rhsz);
    casa::objcopy (itsData + outm + rhsz*itsSize, tmpcolPtr, lhsz, itsSize, 1);
    casa::objcopy (itsData + outm, tmpcolPtr + lhsz, rhsz, itsSize, 1);
    return;
  }

  void FFTCMatrix::flip (const std::complex<float>* __restrict__ in,
                         std::complex<float>* __restrict__ out,
                         bool toZero)
  {
    size_t hsz0 = itsSize/2;
    size_t hsz1 = hsz0;
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
    for (size_t j=0; j<hsz0; ++j) {
      for (size_t i=0; i<hsz0; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
    // q2
    fr = in + hsz0;
    to = out + hsz1*itsSize;
    for (size_t j=0; j<hsz0; ++j) {
      for (size_t i=0; i<hsz1; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
    // q3
    fr = in + hsz0*itsSize;
    to = out + hsz1;
    for (size_t j=0; j<hsz1; ++j) {
      for (size_t i=0; i<hsz0; ++i) {
        to[i] = fr[i];
      }
      fr += itsSize;
      to += itsSize;
    }
    // q4
    fr = in + hsz0*itsSize + hsz0;
    to = out;
    for (size_t j=0; j<hsz1; ++j) {
      for (size_t i=0; i<hsz1; ++i) {
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
    size_t hsz0 = itsSize/2;
    size_t hsz1 = hsz0;
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
    for (size_t j=0; j<hsz0; ++j) {
      for (size_t i=0; i<hsz0; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
    // q2
    fr = in + hsz0;
    to = out + hsz1*itsSize;
    for (size_t j=0; j<hsz0; ++j) {
      for (size_t i=0; i<hsz1; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
    // q3
    fr = in + hsz0*itsSize;
    to = out + hsz1;
    for (size_t j=0; j<hsz1; ++j) {
      for (size_t i=0; i<hsz0; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
    // q4
    fr = in + hsz0*itsSize + hsz0;
    to = out;
    for (size_t j=0; j<hsz1; ++j) {
      for (size_t i=0; i<hsz1; ++i) {
        to[i] = fr[i] * factor;
      }
      fr += itsSize;
      to += itsSize;
    }
  }

  static int fftsizes[] =
      {     1,     3,     5,     7,     9,    11,    13,    15,    21,    25,
           27,    33,    35,    39,    45,    49,    55,    63,    65,    75,
           77,    81,    91,    99,   105,   117,   125,   135,   147,   165,
          175,   189,   195,   225,   231,   243,   245,   273,   275,   297,
          315,   325,   343,   351,   375,   385,   405,   441,   455,   495,
          525,   539,   567,   585,   625,   637,   675,   693,   729,   735,
          819,   825,   875,   891,   945,   975,  1029,  1053,  1125,  1155,
         1215,  1225,  1323,  1365,  1375,  1485,  1575,  1617,  1625,  1701,
         1715,  1755,  1875,  1911,  1925,  2025,  2079,  2187,  2205,  2275,
         2401,  2457,  2475,  2625,  2673,  2695,  2835,  2925,  3087,  3125,
         3159,  3185,  3375,  3465,  3645,  3675,  3773,  3969,  4095,  4125,
         4375,  4455,  4459,  4725,  4851,  4875,  5103,  5145,  5265,  5625,
         5733,  5775,  6075,  6125,  6237,  6561,  6615,  6825,  6875,  7203,
         7371,  7425,  7875,  8019,  8085,  8125,  8505,  8575,  8775,  9261,
         9375,  9477,  9555,  9625, 10125, 10395, 10935, 11025, 11319, 11375,
        11907, 12005, 12285, 12375, 13125, 13365, 13377, 13475, 14175, 14553,
        14625, 15309, 15435, 15625, 15795, 15925, 16807, 16875, 17199, 17325,
        18225, 18375, 18711, 18865, 19683, 19845, 20475, 20625, 21609, 21875,
        22113, 22275, 22295, 23625, 24057, 24255, 24375, 25515, 25725, 26325,
        26411, 27783, 28125, 28431, 28665, 28875, 30375, 30625, 31185, 31213,
        32805, 33075, 33957, 34125, 34375, 35721, 36015, 36855, 37125, 39375,
        40095, 40131, 40425, 40625, 42525, 42875, 43659, 43875, 45927, 46305,
        46875, 47385, 47775, 48125, 50421, 50625, 51597, 51975, 54675, 55125,
        56133, 56595, 56875, 59049, 59535, 60025, 61425, 61875, 64827, 65625,
        66339, 66825, 66885, 67375, 70875, 72171, 72765, 73125, 76545, 77175,
        78125, 78975, 79233, 79625, 83349, 84035, 84375, 85293, 85995, 86625,
        91125, 91875, 93555, 93639, 94325, 98415, 99225
      };

  const int* FFTCMatrix::getOptimalOddFFTSizes()
  {
    return fftsizes;
  }

 int FFTCMatrix::nOptimalOddFFTSizes()
  {
    return sizeof(fftsizes) / sizeof(int);
  }

  int FFTCMatrix::optimalOddFFTSize (int size)
  {
    // Define all odd FFT sizes < 100000 matching the FFTW rule for a good size:
    //     2^a * 3^b * 5^c * 7^d * 11^e * 13^f   with e+f<=1
    // The following python script was used to find them.
    //nrs=[]
    //for n in range(100000):
    //    m=n
    //    ok=(n%2!=0)
    //    n1113=0
    //    while ok and m>3:
    //        if m%3==0:
    //            m=m/3
    //        elif m%5==0:
    //            m=m/5
    //        elif m%7==0:
    //            m=m/7
    //        elif m%11==0:
    //            m=m/11
    //            n1113+=1
    //        elif m%13==0:
    //            m=m/13
    //            n1113+=1
    //        else:
    //            ok=False
    //        if n1113>1:
    //            ok=False
    //    if ok:
    //        nrs.append(n)
    //print len(nrs)
    //print nrs
    if (size >= fftsizes[sizeof(fftsizes)/sizeof(int) - 1]) {
      return (size/2)*2 + 1;   // make odd
    }
    int* bound = std::lower_bound (fftsizes,
                                   fftsizes + sizeof(fftsizes)/sizeof(int),
                                   size);
    return *bound;
  }
    
} //# end namespace

