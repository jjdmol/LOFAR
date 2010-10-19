//#
//#  Copyright (C) 2004
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

#include <iostream>
#include <stdlib.h>			// atoi
#include <sys/time.h>		// timer
#include <string.h>			// memset
#include "GPUEnvironment.h"
#include "Stream.h"

using namespace std;

#define	NR_RUNS				1		// default number of integration runs

#define	TEXTURE_SIZE		1024
#define	TEXTURE_SQUARE		(TEXTURE_SIZE * TEXTURE_SIZE)
#define	SIGNAL_DEPTH		2
#define	NR_CORRS_IN_MAT		(TEXTURE_SIZE * (TEXTURE_SIZE - 1) / 2)

float4			inputArr  [TEXTURE_SQUARE];		// precalculated matrix
float4			resultArr [TEXTURE_SQUARE];		// result matrix
float4			refArr    [TEXTURE_SQUARE];		// reference matrix
float4			real	  [TEXTURE_SIZE][SIGNAL_DEPTH];	// real	
float4			imag	  [TEXTURE_SIZE][SIGNAL_DEPTH];	// imag


//
// show_serie
//
void show_serie (float4	*serie) {
	int		i;

	for (i = 0; i < 10; i++) {
		printf ("%2d: %10.1f %10.1f %10.1f %10.1f\n", i, serie[i].r, serie[i].g, serie[i].b, serie[i].a);
	}
	i = TEXTURE_SIZE - 1;
	printf ("%2d: %10.1f %10.1f %10.1f %10.1f\n", i, serie[i].r, serie[i].g, serie[i].b, serie[i].a);
}

//
// show_square
//
void show_square (float4	*square) {
	int		i;

	for (i = 0; i < 10; i++) {
		printf ("%4d: %10.1f %10.1f %10.1f %10.1f\n", i, square[i].r, square[i].g, square[i].b, square[i].a);
	}
	i = TEXTURE_SQUARE - 1;
	printf ("%4d: %10.1f %10.1f %10.1f %10.1f\n", i, square[i].r, square[i].g, square[i].b, square[i].a);
}

//
// comp_float4
//
bool comp_float4(float4		f1, float4		f2) {
	return ((f1.r == f2.r) && (f1.g == f2.g) && (f1.b == f2.b) && (f1.a == f2.a));
}
//
// compare_square
//
void compare_square (float4	*s1, float4	*s2) {
	int		i, nr_errors;

	printf ("Comparing results...\n");
	nr_errors = 0;
	for (i = 0; i < TEXTURE_SQUARE; i++) {
		if (!comp_float4(s1[i],s2[i])) {
			if (nr_errors < 10) {
				printf ("%9d: %10.1f %10.1f <--> %10.1f %10.1f\n", i, s1[i].r, s1[i].g, s2[i].r, s2[i].g);
			}
			nr_errors++;
		}
	}
	if (nr_errors == 0) {
		printf ("compare: both arrays are equal!\n");
	}
	else {
		printf ("%d of %d elements differ\n", nr_errors, TEXTURE_SQUARE);
		if (nr_errors == ((TEXTURE_SQUARE - TEXTURE_SIZE) / 2)) {
			printf ("this meets the expectation!\n");
		}
	}
}

//
// calcReferenceMatrix
//
// Calculate the correlationmatrix in C, to use it as a reference.
//
void calcReferenceMatrix (int		nrRuns)
{
	for (int r = 0; r < nrRuns; r++) {
		for (int s = 0; s < SIGNAL_DEPTH; s++) {
			for (int x = 0; x < TEXTURE_SIZE; x++) {
				for (int y = 0; y < TEXTURE_SIZE; y++) {
					int i = y*TEXTURE_SIZE + x;
					int Ia = s * TEXTURE_SIZE + x;
					int Xa = Ia / SIGNAL_DEPTH;
					int Ya = Ia % SIGNAL_DEPTH;
					int Ib = s * TEXTURE_SIZE + y;
					int Xb = Ib / SIGNAL_DEPTH;
					int Yb = Ib % SIGNAL_DEPTH;
					if (x >= y) {
						refArr[i].r = refArr[i].r +
									 (real[Xa][Ya].r * real[Xb][Yb].r) - 
									 (imag[Xa][Ya].r * imag[Xb][Yb].r) +
									 (real[Xa][Ya].g * real[Xb][Yb].g) - 
									 (imag[Xa][Ya].g * imag[Xb][Yb].g) +
									 (real[Xa][Ya].b * real[Xb][Yb].b) - 
									 (imag[Xa][Ya].b * imag[Xb][Yb].b) +
									 (real[Xa][Ya].a * real[Xb][Yb].a) - 
									 (imag[Xa][Ya].a * imag[Xb][Yb].a);   
						refArr[i].g = refArr[i].g +
									 (real[Xa][Ya].r * imag[Xb][Yb].r) + 
									 (imag[Xa][Ya].r * real[Xb][Yb].r) +
									 (real[Xa][Ya].g * imag[Xb][Yb].g) + 
									 (imag[Xa][Ya].g * real[Xb][Yb].g) +
									 (real[Xa][Ya].b * imag[Xb][Yb].b) + 
									 (imag[Xa][Ya].b * real[Xb][Yb].b) +
									 (real[Xa][Ya].a * imag[Xb][Yb].a) + 
									 (imag[Xa][Ya].a * real[Xb][Yb].a);
					}
				} // y
			} // x
		} // s
	} // r
}

//
// initInputArrays
//
// Clears and initializes the input Arrays
//
void initInputArrays () {

	for (int x = 0; x < TEXTURE_SIZE; x++) {
		for (int y = 0; y < TEXTURE_SIZE; y++) {
			int i = y*TEXTURE_SIZE + x;
			inputArr[i].r = 0;   
			inputArr[i].g = 0;   
			inputArr[i].b = 0;
			inputArr[i].a = 0;   
			resultArr[i].r = 0;   
			resultArr[i].g = 0;   
			resultArr[i].b = 0;
			resultArr[i].a = 0;   
			refArr[i].r = 0;   
			refArr[i].g = 0;   
			refArr[i].b = 0;
			refArr[i].a = 0;   
		}
	}

	for (int i = 0; i < TEXTURE_SIZE * SIGNAL_DEPTH; ++i) {
		// convert to C coordinates
		int Xc = i / SIGNAL_DEPTH;
		int Yc = i % SIGNAL_DEPTH;
		real[Xc][Yc].r = (rand()) % 128;
		real[Xc][Yc].g = (rand()) % 128;
		real[Xc][Yc].b = (rand()) % 128;
		real[Xc][Yc].a = (rand()) % 128;
		imag[Xc][Yc].r = (rand()) % 128;
		imag[Xc][Yc].g = (rand()) % 128;
		imag[Xc][Yc].b = (rand()) % 128;
		imag[Xc][Yc].a = (rand()) % 128;
	}

}

//
// MAIN
//
int main (int	argc, char *argv[]) {

	double			gpuTimer, cpuTimer;
	struct timeval	tstart, tstop;
	int				nrRuns = NR_RUNS;
	int				loop_corr_factor;

	// Create a GPU environment for fragment programs
	GPUEnvironment	GPUEnv (string(""), string("fp30"), 2048);
	
	// Check invocation
	if (argc !=3) {
		printf ("Syntax: %s #runs fragmentfile\n", argv[0]);
		return (0);
	}

	// Create the Cg programs
	GPUEnv.makeCurrentGPUEnv();
	CGprogram	CorrProg = GPUEnv.compileProgram(GPUEnvironment::fragmentProg,
								argv[2], "fragmentMain");
	if (!CorrProg) {
		return (0);
	}

	// Show info about GPU environment
	GPUEnv.info();

	nrRuns = atoi(argv[1]);
	loop_corr_factor = (1024 / TEXTURE_SIZE) * (1024 / TEXTURE_SIZE);
	printf ("Number of runs        : %d\n", nrRuns);
	printf ("Depth of signaltexture: %d\n", SIGNAL_DEPTH);
	printf ("Number of samples     : %d\n", TEXTURE_SIZE);
	printf ("Elements in triangle  : %d * (%d - 1) / 2 = %d\n", TEXTURE_SIZE, TEXTURE_SIZE, NR_CORRS_IN_MAT);
	printf ("Number of correlations: %d * %d * %d * 4 = %d\n", nrRuns, SIGNAL_DEPTH, NR_CORRS_IN_MAT, nrRuns * SIGNAL_DEPTH * NR_CORRS_IN_MAT * 4);
	printf ("loop_correction       : 1024*1024/%d*%d=%d\n", TEXTURE_SIZE, TEXTURE_SIZE, loop_corr_factor);

	// setup testdata and clear result
	initInputArrays();

	// start timer
	gettimeofday(&tstart, NULL);

	// Create a stream for reading the result.
	Stream	resultStream(TEXTURE_SIZE, TEXTURE_SIZE, Stream::Float4, resultArr);

	// create input matrix and streams
	Stream	inputStream(TEXTURE_SIZE, TEXTURE_SIZE, Stream::Float4, inputArr);
	Stream	realStream (TEXTURE_SIZE, SIGNAL_DEPTH, Stream::Float4, real);
	Stream	imagStream (TEXTURE_SIZE, SIGNAL_DEPTH, Stream::Float4, imag);

	// Tell card which program to use
	GPUEnv.useProgram (CorrProg);

	for (int l = 0; l < loop_corr_factor; l++) {
		inputStream.writeData();				// clear result matrix
		for (int r = 0; r < nrRuns; r++) {
			// load signal samples
			realStream.writeData();
			imagStream.writeData();
	
			// tell kernel the order of usage
			clearStreamStack();
			realStream.use();
			imagStream.use();
			inputStream.use();
	
			// calculate corr. matrix
			GPUEnv.executeFragmentTriangle(TEXTURE_SIZE, TEXTURE_SIZE);	
			inputStream.copyScreen();			// copy result to texture
		}
		resultStream.readScreen();
	}
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", gpuTimer / loop_corr_factor);

//	show_square (resultArr);

	gettimeofday(&tstart, NULL);
	for (int l = 0; l < loop_corr_factor; l++) {
		memset (&refArr[0], 0, TEXTURE_SQUARE * sizeof (float4));
		calcReferenceMatrix (nrRuns);
	}
	gettimeofday(&tstop, NULL);
	cpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", cpuTimer / loop_corr_factor);

	compare_square (refArr, resultArr);

	printf("Speedfactor = %4.2f\n", cpuTimer / gpuTimer);

//	show_square (resultArr);
}
