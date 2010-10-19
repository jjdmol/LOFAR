#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include "PixelBuffer.h"
#include "GPUEnvironment.h"
#include "Stream.h"

using namespace std;

#define NFACTORIAL		12
#define PI  			3.141592653589793
#define PI2 			6.283185307179586
#define NR_STATIONS		10
#define NR_INTERVALS	1024 // 2048
#define	NR_FREQS		2048 // 4096

int factorial[NFACTORIAL] = {1, 2, 6, 24, 120, 720, 5040, 
			     40320, 362880, 3628800, 39916800,
			     479001600};
double taylorcf[NFACTORIAL];

void initMath(void) {
  int i, sign;
  sign=1;
  for (i=0; i<NFACTORIAL; ++i) {
    if (i%2 == 1) sign = -sign;
    taylorcf[i] = sign*1.0/factorial[i];
  }
}


inline double mycos (double x) {
  double x2;
  double sum, x2n;
  int i, sign;
  /* cos(-x)=cos(x) */
  if (x<0) 
    x = -x;
  /* compute x mod (2*pi) */
  if (x > PI2) {
    x = x - PI2*((int)(x/PI2));
  } 
  /* determine interval */
  if (x <= PI/2.0) {
    sign = 1;
  } else {
    if (x <= PI) {
      x = PI-x;
      sign = -1;
    } else {
      if (x <= 1.5*PI) {
	sign = -1;
	x = x - PI;
      } else {
	sign = 1;
	x = PI2-x;
      }
    }
  }
  /* compute cos on interval [0,PI/2] */
  x2 = x*x;
  sum = 1.0;
  x2n = 1.0;
  for (i=1; i<NFACTORIAL; i+=2) {
    x2n *= x2;
    sum += x2n*taylorcf[i];
  }
  /* correct for sign */
  return sign*sum;
}

inline double mycos2 (double x) {
  double x2;
  double sum, x2n;
  int i, sign;
  /* for mycos2 we must have 0<=x<2*PI */

  /* determine interval */
  if (x <= PI/2.0) {
    sign = 1;
  } else {
    if (x <= PI) {
      x = PI-x;
      sign = -1;
    } else {
      if (x <= 1.5*PI) {
	sign = -1;
	x = x - PI;
      } else {
	sign = 1;
	x = PI2-x;
      }
    }
  }
  /* compute cos on interval [0,PI/2] */
  x2 = x*x;
  sum = 1.0;
  x2n = 1.0;
  for (i=1; i<NFACTORIAL; i+=2) {
    x2n *= x2;
    sum += x2n*taylorcf[i];
  }
  /* correct for sign */
  return sign*sum;
}

inline double mysin (double x) {
  return mycos(x-PI/2.0);
}

inline double mysin2 (double x) {
  if (x <= PI/2.0) {
    x += 3*PI/2;
  } else {
    x -= PI/2;
  }
  return mycos2(x);
}


#define fmod(a,b) (a-(int(a/b)*b))

const double pi = 3.1415926517;
double re1[NR_INTERVALS], im1[NR_INTERVALS];
double re2[NR_INTERVALS], im2[NR_INTERVALS];
double tk1[NR_INTERVALS];
double tk2[NR_INTERVALS];
double resre[NR_FREQS*NR_INTERVALS];
double resim[NR_FREQS*NR_INTERVALS];
double tkre[NR_FREQS*NR_INTERVALS];

void dft1(float* tk, int n, float* re, float* im)
{
  double pi2n = 2*pi/n;
  for (int i=0; i<n; ++i) {
    double tmp = pi2n * tk[i];
    re[i] = cos(tmp);
    im[i] = sin(tmp);
  }
}

void dft2(double* tk, int n, double* re, double* im)
{
  double pi2n = 2*pi/n;
  double pi2 = 2*pi;
  for (int i=0; i<n; ++i) {
    double tmp = pi2n * tk[i];
    re[i] = cos(tmp);
    im[i] = sqrt(1-re[i]*re[i]);
    if (fmod(tmp,pi2) > pi) {
      im[i] = -im[i];
    }
  }
}

void dft3(double* tk, int n, double* re, double* im)
{
  double pi2n = 2*pi/n;
  double pi2 = 2*pi;
  for (int i=0; i<n; ++i) {
    double tmp = pi2n * tk[i];
    double tmp1 = fmod(tmp,pi2);
    re[i] = mycos2(tmp1);
    im[i] = sqrt(1-re[i]*re[i]);
    if (tmp1 > pi) {
      im[i] = -im[i];
    }
  }
}

void dft4(double* tk, int n, double* re, double* im)
{
  double pi2n = 2*pi/n;
  for (int i=0; i<n; ++i) {
    double tmp = pi2n * tk[i];
    re[i] = mycos(tmp);
    im[i] = mysin(tmp);
  }
}


double norm(double* x, int n)
{
  double sum=0;
  for (int i=0; i<n; ++i) {
    sum += fabs(x[i]);
  }
  return sum;
}

inline double mulre (double re1, double im1, double re2, double im2)
{
  return re1*re2 - im1*im2;
}
inline double mulim (double re1, double im1, double re2, double im2)
{
  return re2*im1 + re1*im2;
}
inline double mulcre (double re1, double im1, double re2, double im2)
{
  return re1*re2 + im1*im2;
}
inline double mulcim (double re1, double im1, double re2, double im2)
{
  return re2*im1 - re1*im2;
}

// UVW per second for 4 hours.
struct uvw{
  double u,v,w;
};
struct uvws{
  uvw uvwpos[NR_INTERVALS];
  void fill (double seed);
};
// All UVWs per station.
uvws statuvw[NR_STATIONS];

void uvws::fill(double seed)
{
  for (int i=0; i<NR_INTERVALS; ++i) {
    uvwpos[i].u = seed;
    uvwpos[i].v = 1 + seed/100;
    uvwpos[i].w = seed/10000;
    seed+=1;
  }
}
void filluvw()
{
  for (int i=0; i<NR_STATIONS; ++i) {
    statuvw[i].fill (i);
  }
}

void findbits (const double* data, int n, int& m1, int& m2)
{
  m1 = 0;
  m2 = 0;
  int* p = (int*)data;
  for (int i=0; i<n; ++i) {
    m1 |= *p++;
    m2 |= *p++;
  }
}

// Define the array holding exp(i(ul+vm+wn)) per station per time.
float expre[NR_STATIONS][NR_INTERVALS];
float expim[NR_STATIONS][NR_INTERVALS];
float exparg[NR_INTERVALS];
float3	expreim[NR_STATIONS][NR_INTERVALS];
float4	resreim[NR_INTERVALS][NR_FREQS];

int main(int argc , char *argv[])
{
	struct timeval	tstart, tstop;
	double			gpuTimer;
	GPUEnvironment	GPUEnv (string(""), string("fp30"), 2048);
	GPUEnv.makeCurrentGPUEnv();

	// Check invocation
	if (argc !=3) {
		cerr << "Syntax: " << argv[0] << " DFTprogram E-program" << endl;
		return (0);
	}

	// Create the GL environment
	CGprogram	DFTProgram = GPUEnv.compileProgram (GPUEnvironment::fragmentProg, 
											 argv[1], "fragmentMain");
	CGprogram	ExpProgram = GPUEnv.compileProgram (GPUEnvironment::fragmentProg, 
											 argv[2], "fragmentMain");
	if (!DFTProgram || !ExpProgram) {
		return (0);
	}

	GPUEnv.info();

	cout << "Nr of stations    : " << NR_STATIONS << endl;
	cout << "Nr of frequencies : " << NR_FREQS << endl;
	cout << "Nr of intervals   : " << NR_INTERVALS << endl;

	initMath();
	filluvw();
	const double posl = 0.13;
	const double posm = 0.38;
	const double posn = sqrt(1 - posl*posl - posm*posm);	// 0.915806

	printf ("Calculating dfts\n");
	gettimeofday(&tstart, NULL);
	// Calculate ul+vm+wn per station.
	for (int i=0; i<NR_STATIONS; ++i) {
		for (int j=0; j<NR_INTERVALS; ++j) {
			exparg[j] = statuvw[i].uvwpos[j].u * posl +
                  statuvw[i].uvwpos[j].v * posm +
                  statuvw[i].uvwpos[j].w * posn;
		}
		dft1 (exparg, NR_INTERVALS, &(expre[i][0]), &(expim[i][0]));
	}
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", gpuTimer);

	float3		uvw3 [NR_STATIONS][NR_INTERVALS];
	for (int x = 0; x < NR_STATIONS; x++) {
		for (int y = 0; y < NR_INTERVALS; y++) {
			uvw3[x][y].x = statuvw[x].uvwpos[y].u;
			uvw3[x][y].y = statuvw[x].uvwpos[y].v;
			uvw3[x][y].z = statuvw[x].uvwpos[y].w;
		}
	}
	// define streams
	Stream	uvwStream(NR_STATIONS, NR_INTERVALS, Stream::Float3, uvw3);
	Stream  expStream(NR_STATIONS, NR_INTERVALS, Stream::Float3, expreim);

	printf ("Calculating dfts on GPU\n");
	gettimeofday(&tstart, NULL);
	GPUEnv.useProgram(DFTProgram);
	uvwStream.writeData();				// load uvw data
	clearStreamStack();					// tell kernel order of streams
	uvwStream.use();
	GPUEnv.executeFragmentSquare(NR_STATIONS, NR_INTERVALS);	// execute the program
	expStream.readScreen();				// retrieve the result
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", gpuTimer);
	for (int x =0; x < 5; x++) {
		for (int y =0; y < 5; y++) {
			printf ("%2d,%2d: %f,%f <-> %f,%f\n", x, y, 
					expre[x][y], expim[x][y], expreim[x][y].x, expreim[x][y].y);
		}
	}

	printf ("Calculating time-freq matrix on CPU\n");
	gettimeofday(&tstart, NULL);
	const float f0=2;
	const float df=0.1;
	for (int left=0; left<NR_STATIONS; ++left) {
		std::cout << "left = " << left << std::endl;
		for (int right=0; right<left; ++right) {
			int k=0;
			for (int i=0; i<NR_INTERVALS; i++) {
				resre[k] = mulcre(expre[left][i]*f0, expim[left][i]*f0,
								expre[right][i]*f0, expim[right][i]*f0);
				resim[k] = mulcim(expre[left][i]*f0, expim[left][i]*f0,
								expre[right][i]*f0, expim[right][i]*f0);
				float stepre = mulcre(expre[left][i]*df, expim[left][i]*df,
									expre[right][i]*df, expim[right][i]*df);
				float stepim = mulcim(expre[left][i]*df, expim[left][i]*df,
									expre[right][i]*df, expim[right][i]*df);
				++k;
				for (int ik=1; ik<NR_FREQS; ++ik) {
					resre[k] = mulre(resre[k-1], resim[k-1], stepre, stepim);
					resim[k] = mulim(resre[k-1], resim[k-1], stepre, stepim);
					++k;
				}	// ik
			} // i
		} // right
	} // left
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", gpuTimer);

	Stream  resreimStream(NR_INTERVALS, NR_FREQS, Stream::Float4, resreim);
	clearStreamStack();					// tell kernel order of streams
	expStream.use();					// NR_STATIONS x NR_INTERVALS input
	resreimStream.use();				// NR_INTERVALS x NR_FREQS    output
	GPUEnv.useProgram(ExpProgram);

	printf ("Calculating time-freq matrix on GPU\n");
	gettimeofday(&tstart, NULL);
	for (int left=NR_STATIONS; left>0; --left) {
		std::cout << "left = " << left << std::endl;
		for (int f = 0; f < NR_FREQS; f++) {
			GPUEnv.executeFragmentSquare(NR_INTERVALS, left);
			resreimStream.copyScreen();
		}	
	}
	resreimStream.writeData();		// clear results

	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", gpuTimer);

}
