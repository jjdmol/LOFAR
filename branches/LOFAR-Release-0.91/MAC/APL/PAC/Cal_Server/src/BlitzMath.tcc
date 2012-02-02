//#  -*- mode: c++ -*-
//#  Math.cc: MatLab functions using Bliz arrays 

//#  Copyright (C) 2002-2004
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <blitz/array.h>
#include <complex>
//#include <math.h>
#include <cmath>
#include <fstream>

#include <cpplapack.h>
#include "BlitzMath.h"


//using namespace std;
using namespace blitz;
using namespace CPPL;

namespace LOFAR
{
  namespace BLITZ_MATH
  {

template <typename T>
blitz::Array<T,1> bmMatrixToVector(const blitz::Array<T,2>& x)
{
  int size_x1 = x.rows();
  int size_x2 = x.cols();
  
  blitz::Array<T,1> res(size_x1 * size_x2);
  
  for (int idx1 = 0; idx1 < size_x1; idx1++) {
    res(Range(idx1*size_x1,idx1*size_x1+size_x1-1)) = x(Range::all(), idx1);
  }
  return (res);  
}

template <typename T>
blitz::Array<T,2> bmVectorToMatrix(const blitz::Array<T,1>& x)
{
  int size_res = static_cast<int>(sqrt(static_cast<double>(x.rows())));
  
  blitz::Array<T,2> res(size_res, size_res);
  
  for (int idx1 = 0; idx1 < size_res; idx1++) {
    res(Range::all(),idx1) = x(Range(idx1*size_res,idx1*size_res+size_res-1));
  }
  return (res);
}


// return multiplyd matrix
template <typename T>
blitz::Array<T,2> bmMult(const blitz::Array<T,2>& a, const blitz::Array<T,2>& b)
{
  ASSERT(a.cols() == b.rows());
  firstIndex i;
  secondIndex j;
  thirdIndex k;
  
  blitz::Array<T,2> res( sum(a(i, k) * b(k, j), k));
       
  return (res);
}

// return multiplyd matrix
blitz::Array<std::complex<double>,2> bmMult(const blitz::Array<double,2>& a, const blitz::Array<std::complex<double>,2>& b)
{
  ASSERT(a.cols() == b.rows());
  firstIndex i;
  secondIndex j;
  thirdIndex k;
  
  blitz::Array<std::complex<double>,2> res( sum(a(i, k) * b(k, j), k));
       
  return (res);
}

// return multiplyd matrix
template <typename T>
blitz::Array<T,2> bmMult(const blitz::Array<T,2>& a, const blitz::Array<T,2>& b, const blitz::Array<T,2>& c)
{
  firstIndex i;
  secondIndex j;
  thirdIndex k;
  
  ASSERT(a.cols() == b.rows());
  blitz::Array<T,2> ab( sum(a(i, k) * b(k, j), k));
    
  ASSERT(ab.cols() == c.rows());
  blitz::Array<T,2> abc( sum(ab(i, k) * c(k, j), k));
  
  return (abc);
}

// return sqr of x
template <typename T>
blitz::Array<T,2> bmSqr(const blitz::Array<T,2>& x)
{
  blitz::Array<T,2> res(x.rows(), x.cols());
  res = pow(x,2);
  return (res);
}

// return conj of x
template <typename T>
blitz::Array<T,2> bmConj(const blitz::Array<T,2>& x)
{
  blitz::Array<T,2> res(x.rows(), x.cols());
  res = conj(x);
  return (res);
}

// return conj of x
template <typename T>
blitz::Array<T,1> bmConj(const blitz::Array<T,1>& x)
{
  blitz::Array<T,1> res(x.rows());
  res = conj(x);
  return (res);
}

// return real part of x
template <typename T>
blitz::Array<T,2> bmReal(const blitz::Array<std::complex<T>,2>& x)
{
  blitz::Array<T,2> res(x.rows(), x.cols());
  res = cast<T>(real(x));
  return (res);
}

// return abs of x
template <typename T>
blitz::Array<T,2> bmAbs(const blitz::Array<std::complex<T>,2>& x)
{
  blitz::Array<T,2> res(x.rows(), x.cols());
  res = cast<T>(abs(x));
  return (res);
}

// return upper triagel of matrix
template <typename T>
blitz::Array<T,2> bmTriu(const blitz::Array<T,2>& x, int k = 0)
{
  blitz::Array<T,2> res(x.rows(), x.cols());
  res = 0;
  
  int kk = k;
  int i_first = 0;
  int i_last   = x.rows();
  int j_first = 0;
  int j_last   = x.cols() - 1;
    
  if (k > 0) {
    j_first = k;
    i_last = x.rows() - k;  
  }
    
  for(int i = i_first; i < i_last; i++) {
     res(i,Range(j_first,j_last)) = x(i,Range(j_first,j_last));     
    
    if (kk < 0) {
      kk++;
    } else {
      j_first++;
    }
  }
  return (res);
}


// return lower triagel of matrix
template <typename T>
blitz::Array<T,2> bmTril(const blitz::Array<T,2>& x, int k = 0)
{
  blitz::Array<T,2> res(x.rows(), x.cols());
  res = 0;
  
  int i_first = 0;
  int i_last = x.rows();
  int j_first = 0;
  int j_last = 0;
    
  if (k < 0) {
    i_first = (k * -1);
  } 
  if (k > 0) {
    j_last = k;
  }
    
  for(int i = i_first; i < i_last; i++) {
    res(i,Range(j_first, j_last)) = x(i,Range(j_first, j_last));      
    if (j_last < (x.cols()-1)) { 
      j_last++;
    }
  }
  return (res);
}

// return double matrix with diagonal of one's
template <typename T>
blitz::Array<T,2> bmEye(int m,int n = 0)
{
  if (n == 0) { n = m; }
  blitz::Array<T,2> res(m, n);
  res = 0;
      
  for(int i = 0; i < min(m, n); i++) {
    res(i, i) = 1;      
  }
  return (res);
}

// return new matrix, but m x n times bigger than x
template <typename T>
blitz::Array<T,2> bmRepmat(const blitz::Array<T,2>& x, int m, int n)
{
  int xm = x.rows();
  int xn = x.cols();
  blitz::Array<T,2> res(xm * m, xn * n);
  
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      res(Range(i*xm,i*xm+xm-1),Range(j*xn,j*xn+xn-1)) = x.copy();
    }
  } 
  return (res);
}



// range(first, step, last) command, 
// fills a range from first to last with given step size
// 
template <typename T>
blitz::Array<T,1> bmRange(T first, T step, T last)
{
   int rangeSize = static_cast<int>((last - first + step) / step);

   blitz::Array<T,1> r(rangeSize);
   
   T val = first;
   for (int i = 0; i < rangeSize; i++) {
      r(i) = val;
      val += step;
   }
   return (r);
}


// return a meshgrid
// fill x and y with the range(first, step, last) cmd
// if y = 0, then y = x
template <typename T>
blitz::Array<T,2> bmMeshgrid(const blitz::Array<T,1> x)
{
   blitz::Array<T,2> res(x.rows(),x.rows());
      
   for (int i = 0; i < x.rows(); i++) {
      res(i,Range::all()) = x.copy();
   }
   return (res);
}

// Matrix transpose.
//
// trans(x) is the linear algebraic transpose of x.
template <typename T>
blitz::Array<T,2> bmTrans(const blitz::Array<T,2>& x)
{
  blitz::Array<T,2> xx(x.copy());
  xx.transposeSelf(blitz::secondDim, blitz::firstDim);
  return (xx);
}

// Matrix transpose.
//
// trans(x) is the linear algebraic transpose of x.
template <typename T>
blitz::Array<T,2> bmTransH(const blitz::Array<T,2>& x)
{
  blitz::Array<T,2> xx(x.copy());
  xx.transposeSelf(blitz::secondDim, blitz::firstDim);
  xx = conj(xx);  
  return (xx);
}


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

template <typename T> 
blitz::Array<T,1> bmDiag(const blitz::Array<T,2>& x, int k = 0)
{
  // Calculate the number of elements on the k-th diagonal.
  int n;
  if (k < 0) { 
    n = std::max(0, std::min(x.cols(), x.rows() + k));
  } else { 
    n = std::max(0, std::min(x.rows(), x.cols() - k));
  }
  
  // Allocate a vector that will hold the elements of the k-th diagonal.
  blitz::Array<T,1> v(n);
  
  // Assign the elements of the k-th diagonal.
  for (int i = 0; i < n; i++) {
    if (k < 0) { 
      v(i) = x(i - k, i);
    } else { 
      v(i) = x(i, i + k);
    }
  }
  return (v); 
}


template <typename T> 
blitz::Array<T,2> bmDiag(const blitz::Array<T,1>& x, int k = 0)
{
  // Calculate the size of the square matrix to be formed.
  int n = x.rows();
  if (k < 0) { n += -k; }
  else { n += k; }

  // Allocate a matrix that will hold the elements of v.
  blitz::Array<T,2> m(n, n);

  // Set all elements equal to zero.
  m = 0;
  
  // Assign the elements of v to the k-th diagonal.
  for (int i = 0; i < x.size(); i++) {
    if (k < 0) { 
      m(i-k, i) = x(i);
    } else { 
      m(i, i+k) = x(i);
    }
  }
  return (m);  
}


// Average or mean value.
//
// For vectors, mean(x) is the mean value of the elements in x. For
// matrices, mean(x) is a vector containing the mean value of each column.
template <typename T> 
T bmMean(const blitz::Array<T,1>& x)
{
  // the mean function of blitz will give divide errors with complex values
  return (sum(x) / static_cast<T>(x.rows()));
}

template <typename T> 
blitz::Array<T,1> bmMean(const blitz::Array<T,2>& x)
{
  blitz::firstIndex i;
  blitz::secondIndex j;
  //# We must calculate the column-wise mean, hence we should "reduce"
  //# using the second index.
  // the mean function of blitz will give divide errors with complex values
  blitz::Array<T,1> res(sum(x(j,i),j) / static_cast<T>(x.rows()));
  return (res);
}

// Median value.
//
// For vectors, median(x) is the median value of the elements in x. For
// matrices, median(x) is a vector containing the median value of each
// column.
template <typename T> 
T bmMedian(const blitz::Array<T,1>& x)
{
  T res;
  blitz::Array<T,1> xx(x.copy());
  int d1 = xx.rows();
  bool swapped;
  
  do {
    swapped = false; 
    
    for (int i = 0; i < d1-1; i++) {
      if (abs(xx(i)) > abs(xx(i+1))) {
        swap(xx(i),xx(i+1));
        swapped = true;
      }
    }
    
  } while(swapped);  
  
  int m = (d1 / 2); // get middle of array    
  if ((d1 % 2) == 0) {
    res = (xx(m-1) + xx(m)) / static_cast<T>(2);
  } else {
    res = xx(m);
  }
  
  return (res);
}

template <typename T> 
blitz::Array<T,1> bmMedian(const blitz::Array<T,2>& x)
{
  // Allocate a vector that will hold the median values.
  blitz::Array<T,1> v(x.cols());
  blitz::Array<T,1> x1(x.rows());
  
  for (int i = 0; i < x.cols(); i++) {
    x1 = x(blitz::Range::all(),i); 
    v(i) = bmMedian<T>(x1);
  }
  return (v);
}

// Variance. 
//
// For vectors, var(x) returns the variance of x. For matrices, var(x)
// is a vector containing the variance of each column of x.
// 
// var(x) normalizes by N-1 where N is the sequence length. This makes
// var(x) the best unbiased estimate of the variance if x is a sample
// from a normal distribution.
template <typename T> 
double bmVar(const blitz::Array<T,1>& x)
{
  blitz::Array<T,1> xx(x.copy());
  return (sum(sqr(abs(xx - bmMean(xx)))) / (std::max(1, x.rows()) - 1));
}

template <typename T> 
blitz::Array<double,1> bmVar(const blitz::Array<T,2>& x)
{
  blitz::Array<double,2> xx(abs(x));
  //blitz::firstIndex i;
  //blitz::secondIndex j;
  // We must calculate the column-wise mean, hence we should "reduce"
  // using the second index.
  blitz::Array<T,1> cm(bmMean(x));
  
  // Allocate a vector that will hold the variances.
  blitz::Array<double,1> v(x.cols());

  for (int i = 0; i < x.cols(); i++) {
    
    v(i) = sum( sqr( abs( x(blitz::Range::all(),i) - cm(i)))) / (std::max(1, x.rows()) - 1);
  }
  return (v);
}

// return double kronecker product of matrix
template <typename T>
blitz::Array<T,2> bmKron(const blitz::Array<T,2>& x, blitz::Array<T,2>& y)
{
  blitz::Array<T,2> res( x.rows() * y.rows(),
                        x.cols() * y.cols() );
  res = 0;
                          
  int x1 = x.rows();
  int x2 = x.cols();
  int y1 = y.rows();
  int y2 = y.cols();
  
  for (int a = 0; a < x1; a++) {
    for (int b = 0; b < x2; b++) {
      res(Range(a*y1,a*y1+y1-1), Range(b*y2,b*y2+y2-1)) = x(a, b) * y;
    }
  } 
  return (res);
}

// return double kronecker product of matrix
template <typename T>
blitz::Array<T,2> bmKhatrirao(const blitz::Array<T,2>& x, const blitz::Array<T,2>& y)
{
  blitz::Array<T,2> res( x.rows() * y.rows(), x.cols() );
  res = 0;
                          
  int x1 = x.rows();
  int x2 = x.cols();
  int y1 = y.rows();
  
  for (int idx1 = 0; idx1 < x2; idx1++) {
    for (int idx2 = 0; idx2 < x1; idx2++) {
      res(Range(idx2*y1,idx2*y1+y1-1), idx1) = x(idx2,idx1) * y(Range::all(),idx1);
    }
  } 
  return (res);
}


// return standard deviation of a matrix
// flag = 0, normalize by n-1
// flag = 1, normalize by normalize
// dim = 1, std of each column
// dim = 2, std of each row
template <typename T>
blitz::Array<double,1> bmStd(const blitz::Array<T,2>& x, int flag = 0, int dim = 1 )
{
  blitz::Array<double,1> res(x.cols());
  blitz::Array<std::complex<double>,1> xsum(x.cols());
  blitz::Array<std::complex<double>,2> xx(cast<std::complex<double> >(x));
  std::complex<double> meanval, s;
  int size_a, size_b;
  
  res = 0;
  
  if ((flag < 0) || (flag > 1) || (dim < 1) || (dim > 2)) {
    return (res);
  }
  
  if (dim == 1) {
    size_a = x.cols();
    size_b = x.rows();
  } else {
    size_a = x.rows();
    size_b = x.cols();
  }
  
  firstIndex i;
  secondIndex j;
  if (dim == 1) {
      xsum = sum( xx(j,i), j);
  } else {
      xsum = sum( xx(i,j), i);
  }
  for (int a = 0; a < size_a; a++) {
    // determine mean
    meanval = xsum(a) / static_cast<std::complex<double> >(size_b);
    // determine std
    s = 0;
    for (int b = 0; b < size_b; b++) {
      if (dim == 1) {
        s += (xx(b, a) - meanval) * (xx(b, a) - meanval);  
      } else {
        s += (xx(a, b) - meanval) * (xx(a, b) - meanval);  
      } 
    }
    res(a) = sqrt(abs(s) / (size_b - 1 + flag));
  }
  return (res);
}

// CPPL
//
// the following functions use cpplapack 
// and are only availble for type complex<double>
//
// CPPL 

// return eigenvector en eigenvalue van matrix
bool bmEig(const blitz::Array<std::complex<double>,2>& x, 
               blitz::Array<std::complex<double>,2>& v, 
               blitz::Array<std::complex<double>,2>& d )
{
  int d1 = x.rows();
  v.resize(d1,d1);
  d.resize(d1,d1);
  v = 0;
  d = 0;    

  zgematrix xlapack(d1, d1);
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d1; j++) {
      xlapack(i,j) = x(i, j);
    }
  }
  
  vector<std::complex<double> > eigenval;
  vector<zcovector> eigenvec;     
  
  xlapack.zgeev(eigenval, eigenvec);   
   
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d1; j++) {
      v(i, j) = eigenvec[j](i);
      if ( i == j) d(i,j) = eigenval[i];   
    }
  }
  return (true);
}


// return svd of matrix
bool bmSvd(const blitz::Array<std::complex<double>, 2>& x, 
               blitz::Array<std::complex<double>, 2>& u, 
               blitz::Array<double, 2>& s, 
               blitz::Array<std::complex<double>, 2>& v )
{
  // svd(x) --> uses LAPACK ZGESVD (double precision complex)
  int d1 = x.rows();
  int d2 = x.cols();
  
  s.resize(d1,d2);
  v.resize(d1,d2);
  u.resize(d1,d2);
  s = 0;
  v = 0;
  u = 0;
  zgematrix xlapack(d1, d2);
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d2; j++) {
      xlapack(i,j) = x(i,j);
    }
  }
  
  dcovector S;
  zgematrix U;
  zgematrix VT;
  xlapack.zgesvd(S, U, VT);  
      
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d2; j++) {
      u(i,j) = U(i,j);
      if (i == j) s(i,j) = S(i); 
      v(i,j) = VT(j,i);
    }
  }
  
  return (true);
}


// return norm of matrix
// norm(a)   -->  max(svd(a))
// norm(a,1) -->  max(sum(abs(a)))
// norm(a,2) same as norm(a)
// norm(a,inf) -->  max(sum(abs(a')))
// norm(a,fro) -->  sqrt(sum(diag(a'*a)))
template <typename T>
double bmNorm(const blitz::Array<T,2>& x, int p = 2)
{
    double res = 0; // response value
    firstIndex i;
    secondIndex j;
        
    switch (p)
    {
      case 1: {
        blitz::Array<T,2> xx(x.copy());
        blitz::Array<T,2> h(conj(bmTrans(xx))); // hermite of x
        blitz::Array<double,2> a(abs(h)); // abs of h
        res = blitz::max( sum( a(j,i), j) );
      } break;
      
      case 2: {
        blitz::Array<T,2> xx(cast<std::complex<double> >(x.copy()));
        blitz::Array<std::complex<double>,2> u;
        blitz::Array<double,2> s;
        blitz::Array<std::complex<double>,2> v;

        bmSvd(xx,u,s,v);
        res = blitz::max( bmDiag(s) );
      } break;
      
      case inf: {
        blitz::Array<T,2> xx(x.copy());
        blitz::Array<T,2> h(conj(bmTrans(xx))); // hermite of x
        blitz::Array<double,2> a(abs(h)); // abs of h
        res = max(sum(a(j,i),j));
      } break;
      
      case fro: {
        blitz::Array<T,2> xx(x.copy());
        blitz::Array<T,2> h(bmTransH(xx)); // hermite of x
        res = sqrt( abs( sum( bmDiag( bmMult(h,xx) ) ) ) );
      } break;
      
      default: break;
    }
    return (res);
}

// return inverse of matrix
blitz::Array<std::complex<double>,2> bmInv(const blitz::Array<std::complex<double>, 2>& x)
{
  ASSERT(x.cols() == x.rows());
  
  int d1 = x.rows();
  int d2 = x.cols();
  
  zgematrix xlapack(d1, d2);
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d2; j++) {
      xlapack(i,j) = x(i,j);
    }
  }

  zgematrix xilapack(CPPL::i(xlapack));  
  blitz::Array<std::complex<double>,2> res(d1, d2);
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d2; j++) {
       res(i,j) = xilapack(i,j);
    }
  }
  return (res);
}

// return pseudoinverse of matrix
blitz::Array<std::complex<double>,2> bmPinv(const blitz::Array<std::complex<double>, 2>& x)
{
  blitz::Array<std::complex<double>,2> tmp2( bmInv( bmMult( bmTransH(x), x)));
  blitz::Array<std::complex<double>,2> res(bmMult(tmp2, bmTransH(x)));
  
  return (res);
}
// functions to print array or vector to a file

// functions to print array or vector to a file
template <typename T>
void bmToFile(const blitz::TinyVector<T, 1>& x, string name, int format=0)
{
	if ((format < 1) || (format > 2)) return;
	
	time_t now = time(0);
	struct tm* t = gmtime(&now);
	char filename[PATH_MAX];
    
	snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_%s_%dx",
	t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
	t->tm_hour, t->tm_min, t->tm_sec,
	name.c_str(),
	x.extent(firstDim));

  fstream filestr;
  if (format == 1) {
    strcat(filename,".dat");
    filestr.open (filename, fstream::out | fstream::trunc );
    if (filestr.bad()) {
		  LOG_FATAL_STR("failed to open file: " << filename);
		  exit(EXIT_FAILURE);
	  }
    filestr << x << flush;
  } else {
    strcat(filename,".bin");
    filestr.open (filename, fstream::out | fstream::trunc | fstream::binary);
    if (filestr.bad()) {
		  LOG_FATAL_STR("failed to open file: " << filename);
		  exit(EXIT_FAILURE);
	  }
	  int size = sizeof(T) * x.size();
	  LOG_INFO_STR(name << " size=" << size << " bytes"); 
	  char* xstr = (char*) malloc(size);
    memcpy(xstr,x.data(),size);
    filestr.write(xstr,size);
    free(xstr);
  }

  if (filestr.bad()) {
		LOG_FATAL_STR("failed to write to file: " << filename);
		exit(EXIT_FAILURE);
	}
	filestr.close();
}




template <typename T>
void bmToFile(const blitz::Array<T, 1>& x, string name, int format=0)
{
	if ((format < 1) || (format > 2)) return;
	
	time_t now = time(0);
	struct tm* t = gmtime(&now);
	char filename[PATH_MAX];
    
	snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_%s_%dx",
	t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
	t->tm_hour, t->tm_min, t->tm_sec,
	name.c_str(),
	x.extent(firstDim));

  fstream filestr;
  if (format == 1) {
    strcat(filename,".dat");
    filestr.open (filename, fstream::out | fstream::trunc );
    if (filestr.bad()) {
		  LOG_FATAL_STR("failed to open file: " << filename);
		  exit(EXIT_FAILURE);
	  }
    filestr << x << flush;
  } else {
    strcat(filename,".bin");
    filestr.open (filename, fstream::out | fstream::trunc | fstream::binary);
    if (filestr.bad()) {
		  LOG_FATAL_STR("failed to open file: " << filename);
		  exit(EXIT_FAILURE);
	  }
	  int size = sizeof(T) * x.size();
	  LOG_INFO_STR(name << " size=" << size << " bytes"); 
	  char* xstr = (char*) malloc(size);
    memcpy(xstr,x.data(),size);
    filestr.write(xstr,size);
    free(xstr);
  }

  if (filestr.bad()) {
		LOG_FATAL_STR("failed to write to file: " << filename);
		exit(EXIT_FAILURE);
	}
	filestr.close();
}

template <typename T>
void bmToFile(const blitz::Array<T, 2>& x, string name, int format=0)
{
	if ((format < 1) || (format > 2)) return;
	  
	time_t now = time(0);
	struct tm* t = gmtime(&now);
	char filename[PATH_MAX];
 
	snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_%s_%dx%d",
	t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
	t->tm_hour, t->tm_min, t->tm_sec,
	name.c_str(),
	x.extent(firstDim),
	x.extent(secondDim));
  
  fstream filestr;
  if (format == 1) {
    strcat(filename,".dat");
    filestr.open (filename, fstream::out | fstream::trunc );
    if (filestr.bad()) {
		  LOG_FATAL_STR("failed to open file: " << filename);
		  exit(EXIT_FAILURE);
	  }
    filestr << x << flush;
  } else {
    strcat(filename,".bin");
    filestr.open (filename, fstream::out | fstream::trunc | fstream::binary);
    if (filestr.bad()) {
		  LOG_FATAL_STR("failed to open file: " << filename);
		  exit(EXIT_FAILURE);
	  }
	  int size = sizeof(T) * x.size();
	  LOG_INFO_STR(name << " size=" << size << " bytes"); 
	  char* xstr = (char*) malloc(size);
    memcpy(xstr,x.data(),size);
    filestr.write(xstr,size);
    free(xstr);
  }

  if (filestr.bad()) {
		LOG_FATAL_STR("failed to write to file: " << filename);
		exit(EXIT_FAILURE);
	}
	filestr.close();
}




  } // end namespace BLITZ_MATH
} // end namespace LOFAR
