//  LCSMath.h: Auxiliary functions on Lorrays
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#if !defined(MATH_LCSMATH_H)
#define MATH_LCSMATH_H

//# Includes
#include <Common/Lorrays-Blitz.h>

#ifdef HAVE_FFTW2
# include <rfftw.h>
# include <fftw.h>
#endif

namespace LOFAR
{
  namespace LCSMath
  {
    // Get conjugate of a complex matrix.
    LoMat_dcomplex conj (const LoMat_dcomplex& aMatrix);

    // Get conjugate of a complex vector.
    LoVec_dcomplex conj (const LoVec_dcomplex& aVector);

    // Get hermitian transpose of a complex matrix.
    LoMat_dcomplex hermitianTranspose (const LoMat_dcomplex& aMatrix);

    // Get hilbert transformation of a vector.
    // Needs FFTW.
    LoVec_dcomplex hilbert (const LoVec_double& input);

    // Get k-th diagonal.
    // k==0 means the diagonal starting at a(0,0).
    // K<0 means a diagonal to the left of it; k>0 to the right.
    template<class T> blitz::Array<T,1> diag (const blitz::Array<T,2>&, int k=0);
    template<class T> blitz::Array<T,2> diag (const blitz::Array<T,1>&, int k=0);

    // Sort each column in the matrix.
    LoMat_double sort (const LoMat_double& aMatrix);

    // Sort (in place) the vector in ascending order using quicksort.
    LoVec_double sort (LoVec_double& aVector);

    // Sort an array in ascending order. The sort is done in place.
    void quicksort (unsigned long n, double data[]);

    // Eigen value decomposition and singular value decomposition
    // These operations require some LAPACK routines, which are
    // available from http://dop39/stationSim/liblapack/
    // Programs using these fuctions should be linked with this
    // library.
    //
    // NOTE: Make sure to copy the input matrices, since both lofar_eig::eig and 
    // lofar_eig::svd destroy the values in the input matrix.
    // Needs LAPACK.
    bool is_hermitian (const LoMat_dcomplex& m);
    int eig (const LoMat_dcomplex& m, LoMat_dcomplex& V, LoVec_double& D);
    int hermitian_init (const LoMat_dcomplex& a, LoMat_dcomplex& V,
                        LoVec_double& D);
    int svd (const LoMat_dcomplex& a, LoMat_dcomplex& U,
             LoMat_dcomplex& V, LoVec_double& D);


    // Array Correlation Matrix.
    LoMat_dcomplex acm (const LoMat_dcomplex& a);

    // Multiply matrices or vectors resulting in a matrix.
    LoMat_dcomplex matMult (const LoMat_dcomplex& A, const LoMat_dcomplex& B);
    LoMat_dcomplex matMult (const LoVec_dcomplex& A, const LoVec_dcomplex& B);
    LoMat_double   matMult (const LoMat_double& A, const LoMat_double& B);
    LoMat_double   matMult (const LoVec_double& A, const LoVec_double& B);

    // Invert a matrix
    LoMat_dcomplex  invert (const LoMat_dcomplex& in);

    // Return the complex modulus (magnitude) of the elements of the Vector
    // or Matrix
    LoVec_double absVec (const LoVec_dcomplex& aVec);
    LoMat_double absMat (const LoMat_dcomplex& aMat);

    // Statistics routines
    template <class T> T sum (const blitz::Array <T, 1>& aVector, int length);
    template <class T> T sum_square (const blitz::Array <T, 1>& aVector, int length);
    template <class T> T mean (const blitz::Array <T, 1>& aVector, int length);
    template <class T> T variance (const blitz::Array <T, 1>& aVector, int length);
    template <class T> T stdev (const blitz::Array <T, 1>& aVector, int length);
    template <class T> T median (blitz::Array <T, 1>& aVector, int length);
    template <class T> T max (blitz::Array <T, 1>& aVector, int length);
    template <class T> T min (blitz::Array <T, 1>& aVector, int length);

  } // namespace LCSMath

} // namespace LOFAR

#endif
