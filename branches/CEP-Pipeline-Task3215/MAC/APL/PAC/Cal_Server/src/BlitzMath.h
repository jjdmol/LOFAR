//#  Math.h: MatLab functions using Blitz arrays.
//#
//#  Copyright (C) 2002-2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BLITZ_MATH_H
#define LOFAR_BLITZ_MATH_H

#if !defined(HAVE_BLITZ)
#error Blitz++ package is required
#endif

//# Includes
#include <blitz/array.h>
#include <complex>
//#include <math.h>
#include <cmath>

namespace LOFAR
{
  namespace BLITZ_MATH
  {
  
  template <typename T> blitz::Array<T,1> bmMatrixToVector(const blitz::Array<T,2>& x);
  
  template <typename T> blitz::Array<T,2> bmVectorToMatrix(const blitz::Array<T,1>& x);
  
  template <typename T> blitz::Array<T,2> bmMult(const blitz::Array<T,2>& a, 
                                                 const blitz::Array<T,2>& b );

  blitz::Array<std::complex<double>,2> bmMult(const blitz::Array<double,2>& a, 
                                              const blitz::Array<std::complex<double>,2>& b);
  
  template <typename T> blitz::Array<T,2> bmMult(const blitz::Array<T,2>& a,
                                                 const blitz::Array<T,2>& b, 
                                                 const blitz::Array<T,2>& c );                                              
  
  template <typename T> blitz::Array<T,2> bmSqr(const blitz::Array<T,2>& x);

  template <typename T> blitz::Array<T,2> bmConj(const blitz::Array<T,2>& x);

  template <typename T> blitz::Array<T,1> bmConj(const blitz::Array<T,1>& x);
  
  template <typename T> blitz::Array<T,2> bmReal(const blitz::Array<std::complex<T>,2>& x);
    
  template <typename T> blitz::Array<T,2> bmAbs(const blitz::Array<std::complex<T>,2>& x);

  // return upper triagel of matrix
  template <typename T> blitz::Array<T,2> bmTriu(const blitz::Array<T,2>& x, 
                                               int k = 0 );
  
  // return lower triagel of matrix
  template <typename T> blitz::Array<T,2> bmTril(const blitz::Array<T,2>& x, 
                                                int k = 0 );
 
  // return an eye matrix 
  // with a size of m x n, if n = 0 then it returns a m x m matrix
  template <typename T> blitz::Array<T,2> bmEye(int m,
                                              int n = 0 ) ;
  
  // return new matrix, but m x n times bigger than x
  template <typename T> blitz::Array<T,2> bmRepmat(const blitz::Array<T,2>& x, 
                                                 int m, 
                                                 int n );


   // range(first, step, last) command, 
  // fills a range from first to last with given step size
  // 
  template <typename T> blitz::Array<T,1> bmRange(T first, 
                                                T step, 
                                                T last );
      
  // return a meshgrid xx
  // fill x with the range(first, step, last) cmd
  // xx = meshgrid(x)
  template <typename T> blitz::Array<T,2> bmMeshgrid(const blitz::Array<T,1> x);

  // Matrix transpose.
  //
  // trans(x) is the linear algebraic transpose of x.
  template <typename T> blitz::Array<T,2> bmTrans(const blitz::Array<T,2>& x);

  // Matrix transpose.
  //
  // trans(x) is the linear algebraic transpose of x.
  template <typename T> blitz::Array<T,2> bmTransH(const blitz::Array<T,2>& x);

  
  // Diagonal matrices and diagonals of a matrix.
  //
  // diag(v,k) when v is a vector with n components is a square matrix of
  // order n+abs(k) with the elements of v on the k-th diagonal. k = 0 is
  // the main diagonal, k > 0 is above the main diagonal and k < 0 is below
  // the main diagonal.
  //
  // diag(v) is the same as diag(v,0) and puts v on the main diagonal.
  //
  // diag(x,k) when x is a matrix is a column vector formed from
  // the elements of the k-th diagonal of x.
  template <typename T> blitz::Array<T,1> bmDiag(const blitz::Array<T,2>& x, int k = 0);
  template <typename T> blitz::Array<T,2> bmDiag(const blitz::Array<T,1>& x, int k = 0);


  // Average or mean value.
  //
  // For vectors, mean(x) is the mean value of the elements in x. For
  // matrices, mean(x) is a vector containing the mean value of each column.
  template <typename T> T bmMean(const blitz::Array<T,1>& x);
  template <typename T> blitz::Array<T,1> bmMean(const blitz::Array<T,2>& x);

// Median value.
  //
  // For vectors, median(x) is the median value of the elements in x. For
  // matrices, median(x) is a vector containing the median value of each
  // column.
  //
  // \note median() is currently implemented using the STL algorithm
  // nth_element(). To use this algorithm, we first need to copy the
  // contents of \a x into an STL vector. This is not terribly efficient,
  // but I didn't have the time to reimplement the nth_element() method for
  // VectorDouble. On the other hand, nth_element() modifies the container, so
  // we would have to make a copy anyway, because we don't want to modify
  // the original Vector \a x.
  
  template <typename T> T bmMedian(const blitz::Array<T,1>& x);
  template <typename T> blitz::Array<T,1> bmMedian(const blitz::Array<T,2>& x);
  
 
  // Variance. 
  //
  // For vectors, var(x) returns the variance of x. For matrices, var(x)
  // is a vector containing the variance of each column of x.
  // 
  // var(x) normalizes by N-1 where N is the sequence length. This makes
  // var(x) the best unbiased estimate of the variance if x is a sample
  // from a normal distribution.
  template <typename T> double bmVar(const blitz::Array<T,1>& x);
  template <typename T> blitz::Array<double,1> bmVar(const blitz::Array<T,2>& x);
  
  // return double kronecker product of matrix
  template <typename T> blitz::Array<T,2> bmKron(const blitz::Array<T,2>& x, 
                                                 const blitz::Array<T,2>& y );
  
  template <typename T> blitz::Array<T,2> bmKhatrirao(const blitz::Array<T,2>& x,
                                                      const blitz::Array<T,2>& y );
  
  // return standard deviation of a matrix
  // flag = 0, normalize by n-1
  // flag = 1, normalize by normalize
  // dim = 1, std of each column
  // dim = 2, std of each row
  template <typename T> blitz::Array<double,1> bmStd(const blitz::Array<T,2>& x, 
                                                    int flag = 0, 
                                                    int dim = 1 );
  
  
  // return norm of matrix
  // norm(a)   -->  max(svd(a))
  // norm(a,1) -->  max(sum(abs(a)))
  // norm(a,2) same as norm(a)
  // norm(a,inf) -->  max(sum(abs(a'*a)))
  // norm(a,fro) -->  sqrt(sum(diag(a'*a)))
  //
  #define inf 3
  #define fro 4
  
  template <typename T> double bmNorm(const blitz::Array<T,2>& x, 
                                     int p = 2 );
  
  
  // return eigenvalue of matrix
  // [V,D] = eig(x)
  bool bmEig(const blitz::Array<std::complex<double>,2>& x, 
           blitz::Array<std::complex<double>,2>& v,
           blitz::Array<std::complex<double>,2>& d );
  
   
  // SVD singular Value Decomposition
  // returns U, S and V of matrix x
  // [U,S,V] = svd(x)
  bool bmSvd(const blitz::Array<std::complex<double>, 2>& x, 
           blitz::Array<std::complex<double>, 2>& u, 
           blitz::Array<double, 2>& s, 
           blitz::Array<std::complex<double>, 2>& v );
 
  
  // return inverse of matrix
  blitz::Array<std::complex<double>,2> bmInv(const blitz::Array<std::complex<double>, 2>& x);
  
 
  // return pseudoinverse of matrix
  blitz::Array<std::complex<double>,2> bmPinv(const blitz::Array<std::complex<double>, 2>& x);
  
  template <typename T> void bmToFile(const blitz::TinyVector<T, 1>& x, string name, int format=0);
  template <typename T> void bmToFile(const blitz::Array<T, 1>& x, string name, int format=0);
  template <typename T> void bmToFile(const blitz::Array<T, 2>& x, string name, int format=0);

  } // namespace BLITZ_MATH 
} // namespace LOFAR

#include <BlitzMath.tcc>

#endif
