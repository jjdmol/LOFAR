//  Locksmith's.cc: Auxiliary functions on Lorrays
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

#ifdef HAVE_CONFIG
# include <config.h>
#endif

#include <Math/LCSMath.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>

#ifdef HAVE_FFTW
# include <rfftw.h>
# include <fftw.h>
#endif

// Declare functions in lapack.
#ifdef HAVE_LAPACK
#define zheev zheev_
#define zgesvd zgesvd_
#define zgetrf zgetrf_
#define zgetri zgetri_
extern "C"
{
  int zheev (const char*, const char*, const int&,
	     dcomplex*, const int&, double*, dcomplex*,
	     const int&, double*, int&);

  int zgesvd (const char*, const char*, const int&,
	      const int&, dcomplex*, const int&,
	      double*, dcomplex*, const int&,
	      dcomplex*, const int&, dcomplex*,
	      const int&, double*, int&);

  int zgetrf (int *m, int *n, dcomplex *a, int *lda, 
	      int *ipiv, int *info);

  int zgetri (int *n, dcomplex *a, int *lda, int *ipiv, 
	      dcomplex *work, int *lwork, int *info);
}
#endif

// /* Subroutine */ int zgetrf_(integer *m, integer *n, doublecomplex *a, 
// 	integer *lda, integer *ipiv, integer *info);
 
// /* Subroutine */ int zgetri_(integer *n, doublecomplex *a, integer *lda, 
// 	integer *ipiv, doublecomplex *work, integer *lwork, integer *info);
 


// Define some macros for quicksort.
#define SWAPVALUES(a,b) temp=(a);(a)=(b);(b)=temp;
#define MAXQS 15
#define BUFFERLENGTH 100


namespace LCSMath
{
  LoMat_dcomplex conj (const LoMat_dcomplex& aMatrix)
  {
    return LoMat_dcomplex (2. * real(aMatrix) - aMatrix);
  }


  LoMat_dcomplex hermitianTranspose (const LoMat_dcomplex& aMatrix)
  {
    LoMat_dcomplex tmp(aMatrix);
    tmp.transposeSelf (1, 0);
    return conj(tmp);
  }


  template<class T>
  blitz::Array<T,1> diag (const blitz::Array<T,2>& aMatrix, int k)
  {
    int nnr = aMatrix.rows();
    int nnc = aMatrix.cols ();

    if (k > 0)
      nnc -= k;
    else if (k < 0)
      nnr += k;

    blitz::Array<T, 1> d;

    if (nnr > 0 && nnc > 0)
    {
      int ndiag = (nnr < nnc) ? nnr : nnc;
      d.resize (ndiag);

      if (k > 0)
      {
        for (int i = 0; i < ndiag; i++)
        {
          d(i) = aMatrix(i, i+k);
        }
      }
      else if ( k < 0)
      {
        for (int i = 0; i < ndiag; i++)
        {
          d(i) = aMatrix(i-k, i);
        }
      }
      else
      {
        for (int i = 0; i < ndiag; i++)
        {
          d(i) = aMatrix(i, i);
        }
      }
    }
    else
        cerr << "diag: requested diagonal out of range\n";

    return d;
  }


  template<class T>
  blitz::Array<T,2> diag (const blitz::Array<T,1>& aVector, int k)
  {
    int size = aVector.size() + abs(k);
    blitz::Array<T, 2> d(size, size);
    if (k > 0) {
      for (int i = 0; i < size; i++) {
	d(i, i + k) = aVector(i);
      }
    } else if ( k < 0) {
      for (int i = 0; i < size; i++) {
	d(i - k, i) = aVector(i);
      }
    } else {
      for (int i = 0; i < size; i++) {
	d(i, i) = aVector(i);
      }
    }
    return d;
  }


  LoMat_double sort (const LoMat_double& aMatrix)
  {
    LoMat_double sorted_matrix(aMatrix.extent(0), aMatrix.extent(1));
    LoVec_double column_vector(aMatrix.extent(1));
    for (int i = aMatrix.rows(); i > 0; --i)
    {
      column_vector = aMatrix(i-1, blitz::Range::all());
      quicksort (column_vector.size(), column_vector.data() - 1);
      sorted_matrix(i-1, blitz::Range::all()) = column_vector;
    }
    return sorted_matrix;
  }


  LoVec_double sort (LoVec_double& aVector)
  {
    Assert (aVector.isStorageContiguous());
    quicksort(aVector.size(), aVector.data() - 1);
    return aVector;
  }


  void quicksort (unsigned long length, double data[])
  {
    unsigned long i, ir = length, j, k, l = 1;
    int buffer_pointer = 0;
    double a, temp;

    vector<unsigned long> buffer(BUFFERLENGTH);

    for (;;)
    {
      if (ir - l < MAXQS)
      {
        for (j = l + 1; j <= ir; j++)
        {
          a = data[j];

          for (i = j - 1; i >= l; i--)
          {
            if (data[i] <= a)
              break;

            data[i + 1] = data[i];
          }

          data[i + 1] = a;
        }

        if (buffer_pointer == 0)
          break;

        ir = buffer[buffer_pointer--];
        l = buffer[buffer_pointer--];
      }
      else
      {
        k = (l + ir) >> 1;
	SWAPVALUES(data[k], data[l+1]);
        if (data[l] > data[ir])
        {
          SWAPVALUES(data[l], data[ir]);
        }

        if (data[l + 1] > data[ir])
        {
          SWAPVALUES(data[l + 1], data[ir]) ;
        }

        if (data[l] > data[l + 1])
        {
          SWAPVALUES(data[l], data[l + 1]);
        }

        i = l + 1;
        j = ir;
        a = data[l + 1];

        for (;;)
        {
          do
          {
            i++;
          }
          while (data[i] < a);

          do
          {
            j--;
          }
          while (data[j] > a);

          if (j < i)
            break;

          SWAPVALUES(data[i], data[j]);
        }

        data[l + 1] = data[j];
        data[j] = a;
        buffer_pointer += 2;

        Assert (buffer_pointer < BUFFERLENGTH);
	if (ir - i + 1 >= j - l)
        {
	  buffer[buffer_pointer] = ir;
	  buffer[buffer_pointer - 1] = i;
	  ir = j - 1;
	}
	else
        {
	  buffer[buffer_pointer] = j - 1;
	  buffer[buffer_pointer - 1] = l;
	  l = i;
	}
      }
    }
  }


  LoVec_dcomplex hilbert (const LoVec_double& input)
  {
#ifdef HAVE_FFTW
    int npts = input.size();
    int centre;
    rfftwnd_plan fftplancomplex_forward = rfftwnd_create_plan(1, &npts, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
    fftw_plan fftplancomplex_backward = fftw_create_plan(npts, FFTW_BACKWARD, FFTW_ESTIMATE);
    LoVec_dcomplex output(npts);

    rfftwnd_one_real_to_complex(fftplancomplex_forward, (fftw_real*)input.data(), (fftw_complex*)output.data());

    if (npts % 2 != 0)
      centre = npts / 2;
    else
      centre = npts / 2 - 1;

    output(blitz::Range(1, centre)) *= 2;
    fftw_one(fftplancomplex_backward, (fftw_complex*)output.data(), (fftw_complex*)output.data());
    output /= npts;

    return output;
#else
    AssertStr (0, "Lorray::hilbert: FFTW is needed but not configured");
#endif
  }


  bool is_hermitian (const LoMat_dcomplex& m)
  {
    int nr = m.rows();
    int nc = m.cols();
    bool t = false;
    if (nr > 0 && nr == nc) {
      t = true;
      LoMat_dcomplex m_trans = hermitianTranspose(m) ; 
      for (int i=0;i<nc;i++){
	for (int j=0;j<nr;j++) {
	  if (m(i,j) != m_trans(i,j)) return false;
	}
      }
    }
    return t;
  }


  int eig (const LoMat_dcomplex& m, LoMat_dcomplex& V, LoVec_double& D)
  {
    AssertStr (is_hermitian(m),
	       "only Hermitian matrices are currently supported");
    return hermitian_init(m, V, D);
  }


  int hermitian_init (const LoMat_dcomplex& a,
		      LoMat_dcomplex& V, LoVec_double& D)
  {
#ifdef HAVE_LAPACK
    // The input matrix is Hermitian <complex<double>>
    // use ZHEEV to calculate the eigen values. 
    int nr = a.rows();
    int nc = a.cols();
    
    AssertStr (nr == nc, "EIG requires square matrix");

    // Define workspace variables for LAPACK routine
    int info = 0;
    LoMat_dcomplex atmp(nc, nr);
    atmp=a;
    dcomplex *tmp_data = atmp.data ();
    
    LoVec_double w (nr) ;
    diag(w,0);
    double *pw = w.data () ; 
    
    int lwork = 8 * nr;
    LoVec_dcomplex work (lwork) ;
    dcomplex *pwork = work.data (); 
    
    int lrwork = 3 * nr;
    LoVec_double rwork (lrwork);
    double *prwork = rwork.data (); 

    // Now apply the Fortran routine from LAPACK
    zheev ("V", "U", nr, tmp_data, nr, pw, pwork,
	   lwork, prwork, info);
    // Once the Fortran routine returns, check the output.
    AssertStr (info == 0, "LoarrayFunc::eig: "
	       << (info<0 ? "invalid ZHEEV argument" : "ZHEEV no converge"));
    D = w;
    V = atmp;
    return info;
#else
    AssertStr (0, "Lorray::eig: LAPACK is needed but not configured");
#endif
  }
  

  int svd (const LoMat_dcomplex& a, LoMat_dcomplex& U,
	   LoMat_dcomplex& V, LoVec_double& D)
    // A       (input/output) COMPLEX*16 array, dimension (LDA,N)
    // On entry, the M-by-N matrix A.
    // On exit, the contents of A are destroyed.
    //
    // U       (output) COMPLEX*16 array, dimension (LDU,UCOL)
    // (LDU,M) if JOBU = 'A' or (LDU,min(M,N)) if JOBU = 'S'.
    // If JOBU = 'A', U contains the M-by-M unitary matrix U;
    // if JOBU = 'S', U contains the first min(m,n) columns of U
    // (the left singular vectors, stored columnwise);
    // if JOBU = 'N' or 'O', U is not referenced.
    //
    // VT      (output) COMPLEX*16 array, dimension (LDVT,N)
    // If JOBVT = 'A', VT contains the N-by-N unitary matrix
    // V**H;
    // if JOBVT = 'S', VT contains the first min(m,n) rows of
    // V**H (the right singular vectors, stored rowwise);
    // if JOBVT = 'N' or 'O', VT is not referenced.
       
    // We provide only a single algorithm for the SVD, although the lapack
    // routine is capable of more variations.
  {
#ifdef HAVE_LAPACK
    int info;
    int m = a.rows ();
    int n = a.cols ();
    
    LoMat_dcomplex atmp = a;
    dcomplex *tmp_data = atmp.data ();
    
    // A should be square in LOFAR, so this is pretty useless
    int min_mn = m < n ? m : n;
    int max_mn = m > n ? m : n;
    
    char jobu = 'A';
    char jobv = 'A';

    int ncol_u = m;
    int nrow_vt = n;
    int nrow_s = m;
    int ncol_s = n;
    
    // left singular values
    dcomplex *u = U.data ();

    // singular values
    double *s_vec = D.data () ;

    // right singular values
    dcomplex *vt = V.data();

    int lwork = 2*min_mn + max_mn;

    LoVec_dcomplex work (lwork);
    dcomplex *pwork = work.data ();
    
    int lrwork = 5*max_mn;

    LoVec_double rwork (lrwork);
    double *prwork = rwork.data ();


    // Apply the corresponding lapack routine
    zgesvd (&jobu, &jobv, m, n, tmp_data, m, s_vec, u,
	    m, vt, nrow_vt, pwork, lwork, prwork, info);
    AssertStr (info==0, "LoarrayFunc::eig: "
	       << (info<0 ? "invalid ZGESVD argument" : "ZGESVD no converge"));
    if (! (jobv == 'N' || jobv == 'O')) {
      V = hermitianTranspose (V);
    }
    return info;
#else
    AssertStr (0, "Lorray::svd: LAPACK is needed but not configured");
#endif
  }


  LoMat_dcomplex acm (const LoMat_dcomplex& a)
  {
    // This assumes the input matrix is row-major. 
    blitz::firstIndex i;
    blitz::secondIndex j;

    int nant = a.rows (); 
    int nsh  = a.cols ();
    int lb   = a.lbound(blitz::secondDim);
    int ub   = a.ubound(blitz::secondDim);

    double alpha = 0; // forgetting factor

    LoVec_dcomplex ones(nant);
    ones = 1;
    LoMat_dcomplex eye = diag(ones); // Identity matrix
    LoMat_dcomplex ACM(nant, nant);

    ACM = (dcomplex) 0;

    for (int k=lb; k<=ub; k++) {
      ACM = (1-alpha) * ACM + 
	alpha * (matMult(a(blitz::Range::all(), k), 
			 a(blitz::Range::all(), k))) ;

//       ACM = ACM + matMult(a(blitz::Range::all(), k), 
// 			  a(blitz::Range::all(), k).
// 			      transpose(blitz::firstDim, blitz::secondDim)) ;
    }
    ACM = ACM / (double)nsh;
    ACM = ACM - eye;
    return ACM;
  }


  LoMat_dcomplex matMult (const LoMat_dcomplex& A, const LoMat_dcomplex& B)
  {
    // Assume row major ordering for now
    int nr = A.rows(); 
    int nc = B.cols();
    
    LoMat_dcomplex out(nr, nc);
    out = 0;
    
    Assert (A.cols() == B.rows());
    for (int k = A.lbound(blitz::firstDim); k <= A.ubound(blitz::firstDim); k++) {
      for (int l = B.lbound(blitz::secondDim); l <= B.ubound(blitz::secondDim); l++) {
	for (int m = A.lbound(blitz::secondDim); m <= A.ubound(blitz::secondDim); m++) {
	  out(k,l) = out(k,l) + A(k,m) * B(m,l);
	}
      }
    }
    return out;
  }
  
  LoMat_dcomplex matMult (const LoVec_dcomplex& A, const LoVec_dcomplex& B)
  {
    // Assume row major ordering for now
    
    int nr = A.size();
    int nc = B.size();
    
    LoMat_dcomplex out(nr, nc);
    out = 0;
    for (int k = A.lbound(blitz::firstDim); k <= A.ubound(blitz::firstDim); k++) {
      for (int l = B.lbound(blitz::firstDim); l <= B.ubound(blitz::firstDim); l++) {
	out(k,l) = out(k,l) + A(k) * B(l);
      }
    }
    return out;
  }
  
  LoMat_double matMult (const LoMat_double& A, const LoMat_double& B)
  {
    // Assume row major ordering for now
    int nr = A.rows(); 
    int nc = B.cols();
    
    LoMat_double out(nr, nc);
    out = 0;
    
    Assert (A.cols() == B.rows());
    for (int k = A.lbound(blitz::firstDim); k <= A.ubound(blitz::firstDim); k++) {
      for (int l = B.lbound(blitz::secondDim); l <= B.ubound(blitz::secondDim); l++) {
	for (int m = A.lbound(blitz::secondDim); m <= A.ubound(blitz::secondDim); m++) {
	  out(k,l) = out(k,l) + A(k,m) * B(m,l);
	}
      }
    }
    return out;
  }
  
  LoMat_double matMult (const LoVec_double& A, const LoVec_double& B)
  {
    // Assume row major ordering for now
    
    int nr = A.size();
    int nc = B.size();
    
    LoMat_double out(nr, nc);
    out = 0;
    for (int k = A.lbound(blitz::firstDim); k <= A.ubound(blitz::firstDim); k++) {
      for (int l = B.lbound(blitz::firstDim); l <= B.ubound(blitz::firstDim); l++) {
	out(k,l) = out(k,l) + A(k) * B(l);
      }
    }
    return out;
  }

  LoMat_dcomplex invert (dcomplex det, const LoMat_dcomplex& in)
  {
#ifdef HAVE_LAPACK
      AssertStr(in.rows() == in.cols(), "The input must be a square matrix!");
      
      int m = in.rows();
      int lda = m; int n = m;               // m, n, lda
      
      LoMat_dcomplex out(in.shape());
      out = in;
  
      LoVec_int ipiv(m);                    // ipiv
      int info;                             // info
      
      zgetrf(&m, &n, out.data(), &lda, ipiv.data(), &info);
      
      if (info == 0) { // LU decomposition worked! 
	  // Calculate the determinate
	  // It is just the product of the diagonal elements
	  det = out(0,0);
	  for (int i = 0; i < n; i++)
	      det *= out(i,i);
	  
	  // Calculate the inverse using back substitution
	  int lwork = 32 * n; // Lazy - we should really get this from ilaenv
	  LoVec_dcomplex work(lwork);
	  zgetri(&m, out.data(), &lda, ipiv.data(), work.data(), &lwork, &info);
      }
      AssertStr(info >= 0, "Illegal argument to getri or getrf");

      return out;
#else
    AssertStr (0, "LCSMath::invert: LAPACK is needed but not configured");
#endif
  }   

  // Explicit instantiations
  template blitz::Array<dcomplex,1> diag(const blitz::Array<dcomplex,2>&, int);
  template blitz::Array<dcomplex,2> diag(const blitz::Array<dcomplex,1>&, int);
  template blitz::Array<double,1> diag(const blitz::Array<double,2>&, int);
  template blitz::Array<double,2> diag(const blitz::Array<double,1>&, int);
}
