//# mpidft.cc:
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <DFTServer/mpidft.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <iostream>

using namespace std;


#define PI  3.141592653589793238462643
#define PI2 6.283185307179586476925286

// light speed
#define C 2.99792458e+08

#define MAXANT 100


#ifdef HAVE_MPI

#include <mpi.h>


#define REQUESTTAG 1
#define TASKTAG    2
#define RESULTTAG  3
#define STOPTAG    4

#define NFACTORIAL 12


int curNrTime=0;
int curNrFreq=0;
double* leftarr=0;
double* rightarr=0;

int factorial[NFACTORIAL] = {1, 2, 6, 24, 120, 720, 5040, 
			     40320, 362880, 3628800, 39916800,
			     479001600};
double taylorcf[NFACTORIAL];
double **resri;


double *makeDoubleArray1D (int len) {
  double *p;
  p = (double*)malloc(len*sizeof(double));
  if (!p) {
    fprintf (stderr, "Fatal error: Out of memory!\n");
    exit(-1);
  }
  return p;
}

double **makeDoubleArray2D (int height, int width) {
  double **p;
  int i;
  p = (double**)malloc(height*sizeof(double *));
  if (!p) {
    fprintf (stderr, "Fatal error: Out of memory!\n");
    exit(-1);
  }
  p[0] = (double*)malloc(width*height*sizeof(double));
  if (!p[0]) {
    fprintf (stderr, "Fatal error: Out of memory!\n");
    free(p);
    p = NULL;
    exit(-1);
  }
  for (i=1; i<height; ++i) 
    p[i] = p[i-1] + width;
  return p;
}

void destroyDoubleArray2D (double **arr) {
  free(arr[0]);
  free(arr);
}


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

void dft(double* tk, int n, double f, double* re, double* im)
{
  int i;
  for (i=0; i<n; ++i) {
    double tmp = PI2 * tk[i] * f;
    re[i] = cos(tmp);
    im[i] = sin(tmp);
    ///    printf ("%lf ",re[i]);
  }
  ///  printf ("  %lf %lf \n",f,pi2);
			///  fflush(stdout);
}


void Task2(int len, double *res, double multRe, double multIm) {
  /* this routine assume re[0] and im[0] are already filled in */
  int i;
  double prevre, previm;
  prevre = res[0];
  previm = res[1];
  for (i=2; i<2*len; i+=2) {
    mulreim(prevre, previm, multRe, multIm, res[i], res[i+1]);
    prevre = res[i];
    previm = res[i+1];
  }
}

void Task (int nrTime, int nrFreq,
	   double *leftv, double *rightv,
	   double *res) {
  int i, k;
  double dre, dim;
  k=0;
  double* leftref0 = leftv;
  double* leftimf0 = leftv+nrTime;
  double* leftredf = leftv+2*nrTime;
  double* leftimdf = leftv+3*nrTime;
  double* rightref0 = rightv;
  double* rightimf0 = rightv+nrTime;
  double* rightredf = rightv+2*nrTime;
  double* rightimdf = rightv+3*nrTime;
  for (i=0; i<nrTime; i++) {
    ///    printf ("left: %lf %lf, right: %lf, %lf\n",leftref0[i],leftimf0[i],rightref0[i],rightimf0[i]);
    ///    fflush(stdout);
    mulcreim (leftref0[i], leftimf0[i],
	      rightref0[i], rightimf0[i],
	      res[k], res[k+1]);
    mulcreim (leftredf[i], leftimdf[i],
	      rightredf[i], rightimdf[i],
	      dre, dim);
    Task2(nrFreq, res+k, dre, dim);
    k+=2*nrFreq;
  }
}

void integrate(double* out, double** in, int dM, int dN, int nM, int nN)
{
  for (int i=0; i<nM; ++i) {
    for (int j=0; j<nN; ++j) {
      double sre=0;
      double sim=0;
      int kl=i*dM;
      int ku=kl+dM;
      int ll=j*dN;
      int lu=ll+dN;
      for (int k=kl; k<ku; ++k) {
        for (int l=ll; l<lu; ++l) {
          sre += in[k][2*l];
          sim += in[k][2*l+1];
        }
      }
      *out++ = sre;
      *out++ = sim;
    }
  }
}

int getTask (int &nrTime, int &nrFreq, double *&leftv, double *&rightv) {
  int task[2];
  MPI_Status stat;

  double t=MPI_Wtime();
  /* request master for a task */
  ///  printf("gettask send request\n");
    ///  fflush(stdout);
  MPI_Send (task, 1, MPI_INT, 0, REQUESTTAG, MPI_COMM_WORLD);
  /* receive task header */
  ///  printf("gettask receive task\n");
    ///  fflush(stdout);
  MPI_Recv (task, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
  if (stat.MPI_TAG == STOPTAG) {
    return 0;  /* All tasks done, return 0 indicating stop */
  }
  
  nrTime = task[0];
  nrFreq = task[1];

  if (nrTime != curNrTime  ||  nrFreq != curNrFreq) {
    curNrTime = nrTime;
    curNrFreq = nrFreq;
    free(leftv);
    free(rightv);
    leftv = (double*)malloc((4*nrTime)*sizeof(double));
    rightv = (double*)malloc((4*nrTime)*sizeof(double));
  }
  ///  printf("gettask receive left,right\n");
						      ///  fflush(stdout);
  MPI_Recv (leftv,  4*nrTime, MPI_DOUBLE, 0, TASKTAG, MPI_COMM_WORLD, &stat);
  MPI_Recv (rightv, 4*nrTime, MPI_DOUBLE, 0, TASKTAG, MPI_COMM_WORLD, &stat);
  t=MPI_Wtime()-t;
#if 0
  printf("gettask time=%lf\n",t);
  fflush(stdout);
  printf("gettask left: %lf %lf, right: %lf %lf\n",leftv[0],leftv[nrTime],
	 rightv[0],rightv[nrTime]);
  fflush(stdout);
#endif
  return 1;
}

void sendTask (int dest, int nrTime, int nrFreq,
	       double *leftv, double *rightv) {
  int task[2];
  task[0] = nrTime;
  task[1] = nrFreq;
  printf("sendtask send task\n");
  fflush(stdout);
  MPI_Send (task, 2, MPI_INT, dest, TASKTAG, MPI_COMM_WORLD);

  ///  printf("sendtask send left: %lf %lf, right: %lf %lf\n",leftv[0],leftv[nrTime],
    ///	 rightv[0],rightv[nrTime]);
    ///  fflush(stdout);
  MPI_Send (leftv,  4*nrTime, MPI_DOUBLE, dest, TASKTAG, MPI_COMM_WORLD);
  MPI_Send (rightv, 4*nrTime, MPI_DOUBLE, dest, TASKTAG, MPI_COMM_WORLD);
}

void Master (int nstat, const int* ant,
	     int nt, int nf, int nint, int ninf,
	     double f0, double df,
	     double l, double m, double n, const double *uvw,
	     int nbaseline, const int* ant1, const int* ant2,
	     double* result) {

  int who, nprocs, msg[2];
  int *count;
  MPI_Request *reqhandle;
  MPI_Request *resulthandle;
  MPI_Status stat;
  int nelt = nt/nint;
  int nelf = nf/ninf;

  // Define array containing indices in other arrays.
  int antindex[MAXANT];
  // Define the array holding exp(i(ul+vm+wn)) per station per time.
  double **expres;
  double* exparg;
  expres = makeDoubleArray2D (nstat, 4*nt);
  exparg = makeDoubleArray1D (nt);

  // compute arguments for the exp(.) function
  for (int i=0; i<nstat; ++i) {
    for (int j=0; j<nt; ++j) {
      exparg[j] = uvw[0]*l + uvw[1]*m + uvw[2]*n;
      ///      printf("exparg-%d-%d %lf %lf %lf %lf\n",i,j,
	///	     uvw[0],uvw[1],uvw[2],exparg[j]);
	///      fflush(stdout);
      uvw += 3;
    }
    dft (exparg, nt, f0, expres[i], expres[i]+nt);
    dft (exparg, nt, df, expres[i]+2*nt, expres[i]+3*nt);
    antindex[ant[i]] = i;
  }
#if 0
  for (int i=0; i<nstat; ++i) {
    printf("v%d",i);
    for (int j=0; j<nt; ++j) {
      printf(" %lf %lf",expres[i][j], expres[i][j+nt]);
    }
    printf("\n");
    fflush(stdout);
  }
#endif

  // perform 'initial' dft

  double startTime = MPI_Wtime();

  /* get number of processes in the process group */
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  /* allocate an array of MPI_Request handles */
  count = (int*)malloc(nprocs*sizeof(int));
  reqhandle = (MPI_Request*)malloc(nprocs*sizeof(MPI_Request));
  resulthandle = (MPI_Request*)malloc(nprocs*sizeof(MPI_Request));
  for (int i=0; i<nprocs; ++i) {
    count[i] = 0;
    resulthandle[i] = MPI_REQUEST_NULL;
  }
  
  int stopcnt=0;
  int left, right, idx=0;
  int flag;
  while (stopcnt != nprocs-1) {
    for (who=1; who<nprocs; ++who) {
      MPI_Iprobe (who, REQUESTTAG, MPI_COMM_WORLD, &flag, &stat);
      if (flag  &&  count[who] < 1) {
	///	printf("master receive msg\n");
	///	fflush(stdout);
	MPI_Recv (&msg, 1, MPI_INT, who, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
        if (idx < nbaseline) {
	  left = antindex[ant1[idx]];
	  right = antindex[ant2[idx]];
	  ///	  printf ("who=%d left=%d right=%d\n", who,left,right);
	    ///	  fflush(stdout);
	  /* send task */
	  sendTask (who, nt, nf, expres[left], expres[right]);
	  /* start receive result */
	  ///	  printf("master receive data\n");
	    ///	  fflush(stdout);
	  MPI_Irecv (result+idx*2*nelt*nelf, 2*nelt*nelf, MPI_DOUBLE, who,
		     RESULTTAG, MPI_COMM_WORLD, &resulthandle[who-1]);
	  count[who]++;
	  idx++;
	} else {
	  ///	  printf("master send stop\n");
	  ///	  fflush(stdout);
	  MPI_Send (&msg, 2, MPI_INT, who, STOPTAG, MPI_COMM_WORLD);
	  stopcnt++;
	}
      }
    }
    MPI_Testany (nprocs-1, resulthandle, &who, &flag, &stat);
    if (flag  &&  who != MPI_UNDEFINED) {
      who = who+1;  /* slaves numbered from 1, master=0 ! */
      ///      printf ("who=%d is ready\n", who);
	///      fflush(stdout);
      count[who]--;
    }
  }
  printf ("duration: %lf\n", MPI_Wtime()-startTime);
  fflush(stdout);
}


void Slave(int nt, int nf, int nint, int ninf) {
  int nelt, nelf, nrTime, nrFreq;

  MPI_Request reqhandle;
  MPI_Status stat;

  /* initialize memory */
  resri = makeDoubleArray2D (nt*2, nf);

  nelt = nt/nint;
  nelf = nf/ninf;

  if (getTask(nrTime, nrFreq, leftarr, rightarr)) {
    /* do computation */
    Task (nrTime, nrFreq, leftarr, rightarr, resri[0]);
    /* integrate result */
    fflush(stdout);
    if (nint > 1 || ninf > 1) {
      integrate(resri[0], resri, nint, ninf, nelt, nelf);
    }
    /* send result to master*/
    printf("send result: %lf %lf\n",resri[0][0], resri[0][1]);
    fflush(stdout);
    MPI_Isend (resri[0], 2*nelt*nelf, MPI_DOUBLE, 0, RESULTTAG, MPI_COMM_WORLD, &reqhandle);

    while (getTask(nrTime, nrFreq, leftarr, rightarr)) {
      /* do computation */
      Task (nrTime, nrFreq, leftarr, rightarr, resri[0]);
      /* integrate result */
      MPI_Wait(&reqhandle, &stat);
      if (nint > 1 || ninf > 1) {
	integrate(resri[0], resri, nint, ninf, nelt, nelf);
      }
      /* send result to master*/
      printf("send result: %lf %lf\n",resri[0][0], resri[0][1]);
      fflush(stdout);
      MPI_Isend (resri[0], 2*nelt*nelf, MPI_DOUBLE, 0, RESULTTAG, MPI_COMM_WORLD, &reqhandle);
    }
    MPI_Wait(&reqhandle, &stat);
  }

  printf("slave done\n");
  fflush(stdout);
  /* release memory */
  free(rightarr);
  printf("slave done1\n");
  fflush(stdout);
  free(leftarr);
  printf("slave done2\n");
  fflush(stdout);
  curNrTime = 0;
  curNrFreq = 0;
  leftarr = 0;
  rightarr = 0;
  destroyDoubleArray2D (resri);
  printf("slave fully done\n");
  fflush(stdout);
}


void doDFTslave (void)
{
  int task[4];

  while (1) {
    /* receive task header */
    MPI_Bcast (task, 4, MPI_INT, 0, MPI_COMM_WORLD);
    if (task[0] <= 0) {
      ///      printf ("Slave ended\n");
      ///      fflush(stdout);
      return;  /* DFT server done, return */
    }
    printf ("Slave %d %d %d %d\n",task[0],task[1],task[2],task[3]);
    fflush(stdout);
    Slave (task[0], task[1], task[2], task[3]);
  }
}

void doDFTmaster (int nstat, const int* ant,
		  int nt, int nf, int nint, int ninf,
		  double f0, double df,
		  double l, double m, double n, const double *uvw,
		  int nbaseline, const int* ant1, const int* ant2,
		  double* result)
{
  int task[4];	
  task[0] = nt;
  task[1] = nf;
  task[2] = nint;
  task[3] = ninf;
  MPI_Bcast (task, 4, MPI_INT, 0, MPI_COMM_WORLD);
  if (nt <= 0) {
    ///    printf ("Master ended\n");
    ///    fflush(stdout);
    return;
  }

  Master (nstat, ant, nt, nf, nint, ninf, f0/C, df/C, l, m, n, uvw,
	  nbaseline, ant1, ant2, result);
}



#else    
// No MPI

#include <complex>

void doDFTmaster (int nstat, const int* ant,
		  int nt, int nf, int nint, int ninf,
		  double f0, double df,
		  double l, double m, double n, const double *uvw,
		  int nbaseline, const int* ant1, const int* ant2,
		  double* result)
{
  // Determine the antennae index.
  int antindex[MAXANT];
  for (int i=0; i<nstat; ++i) {
    antindex[ant[i]] = i;
  }
  complex<double>* cresult = reinterpret_cast<complex<double>*>(result);
  for (int i=0; i<nbaseline; i++) {
    const double* uvw1 = uvw + antindex[ant1[i]] * nt * 3;
    const double* uvw2 = uvw + antindex[ant2[i]] * nt * 3;
    for (int j=0; j<nt; j++) {
      double a1 = uvw1[0]*l + uvw1[1]*m + uvw1[2]*n;
      double a2 = uvw2[0]*l + uvw2[1]*m + uvw2[2]*n;
      double freq = f0/C;
      for (int k=0; k<nf; k++) {
	complex<double> c2(0, PI2 * (a1-a2) * freq);
	*cresult++ = std::exp(c2);
	freq += df/C;
      }
      uvw1 += 3;
      uvw2 += 3;
    }
  }
}

#endif
