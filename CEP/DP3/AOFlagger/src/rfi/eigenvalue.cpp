
#include <AOFlagger/rfi/eigenvalue.h>

#include <stdexcept>

// Needs to be included LAST
#include <AOFlagger/f2c.h>

extern "C" {
	void zheevx_(char *jobz, char *range, char *uplo, integer *n,
							doublecomplex *a, integer *lda, double *vl, double *vu, integer *il,
							integer *iu, double *abtol, integer *nfound, double  *w,
							doublecomplex *z, integer *ldz, doublecomplex *work,
							integer *lwork, doublereal *rwork, integer *iwork, integer *ifail, integer *info);

	void zheev_(char *jobz, char *uplo, integer *n,
							doublecomplex *a, integer *lda, double  *w,
							doublecomplex *work,
							integer *lwork, doublereal *rwork, integer *info);
}

double Eigenvalue::Compute(Image2DCPtr real, Image2DCPtr imaginary)
{
	if(real->Width() != imaginary->Width() || real->Height() != imaginary->Height())
		throw std::runtime_error("Size of real and imaginary don't match in eigen value decomposition");
	if(real->Width() != real->Height())
		throw std::runtime_error("Not a square image given in eigen value decomposition");
	
	char jobz[] = "N";  // compute eigenvalues only
	char range[] = "I"; // the IL-th through IU-th eigenvalues will be found.
	char uplo[] = "U";  // Upper triangle of A is stored
	long int n = real->Width();
	long int lda = n;
	double vl = 0, vu = 0;
	//long int il = 1, iu = 1; // search for first eigenvalue
	long int il = n, iu = n; // search for nth eigenvalue
	double abtol = 0.0;
	long int nfound = 0;
	double w[n];
	doublecomplex z; // for eigenvectors, not used
	long int ldz = 1; // for eigenvectors, not used
	long int ifail = 0;
	long int info = 0;
	
	doublecomplex *a = new doublecomplex[n * n];
	for(int y=0;y<n;++y) {
		for(int x=0;x<n; ++x) {
			a[y*n + x].r = real->Value(x, y);
			a[y*n + x].i = imaginary->Value(x, y);
		}
	}
	
	doublecomplex complexWorkAreaSize;
	long int workAreaSize = -1;
	doublereal *rwork = new doublereal[7*n];
	integer *iwork = new integer[5*n];

	// Determine optimal workareasize
	zheevx_(jobz, range, uplo, &n, a, &lda, &vl, &vu, &il, &iu, &abtol, &nfound, w, &z, &ldz, &complexWorkAreaSize, &workAreaSize, rwork, iwork, &ifail, &info);
	
	if(info != 0)
	{
		delete[] a;
		delete[] rwork;
		delete[] iwork;
		throw std::runtime_error("Can not determine workareasize, zheevx returned an error.");
	}
	
	workAreaSize = (int) complexWorkAreaSize.r;
	doublecomplex *work = new doublecomplex[workAreaSize];
	zheevx_(jobz, range, uplo, &n, a, &lda, &vl, &vu, &il, &iu, &abtol, &nfound, w, &z, &ldz, work, &workAreaSize, rwork, iwork, &ifail, &info);
	
	delete[] work;
	delete[] a;
	delete[] rwork;
	
	if(info != 0)
		throw std::runtime_error("zheevx failed");
	
	return w[0];
}

void Eigenvalue::Remove(Image2DPtr real, Image2DPtr imaginary)
{
	if(real->Width() != imaginary->Width() || real->Height() != imaginary->Height())
		throw std::runtime_error("Size of real and imaginary don't match in eigen value decomposition");
	if(real->Width() != real->Height())
		throw std::runtime_error("Not a square image given in eigen value decomposition");
	
	char jobz[] = "V";  // compute eigenvalues and eigenvectors
	char uplo[] = "U";  // Upper triangle of A is stored
	long int n = real->Width();
	long int lda = n;
	double w[n];
	long int info = 0;
	
	doublecomplex *a = new doublecomplex[n * n];
	for(int y=0;y<n;++y) {
		for(int x=0;x<n; ++x) {
			a[y*n + x].r = real->Value(x, y);
			a[y*n + x].i = imaginary->Value(x, y);
		}
	}
	
	doublecomplex complexWorkAreaSize;
	long int workAreaSize = -1;
	doublereal *rwork = new doublereal[7*n];

	// Determine optimal workareasize
	zheev_(jobz, uplo, &n, a, &lda, w, &complexWorkAreaSize, &workAreaSize, rwork, &info);
	
	if(info != 0)
	{
		delete[] a;
		delete[] rwork;
		throw std::runtime_error("Can not determine workareasize, zheev returned an error.");
	}
	
	workAreaSize = (int) complexWorkAreaSize.r;
	doublecomplex *work = new doublecomplex[workAreaSize];
	zheev_(jobz, uplo, &n, a, &lda, w, work, &workAreaSize, rwork, &info);
	
	delete[] work;
	delete[] a;
	delete[] rwork;
	
	if(info != 0)
		throw std::runtime_error("zheev failed");
		
	for(size_t y=0;y<n;++y)
	{
		for(size_t x=0;x<n;++x)
		{
			
		}
	}
}
