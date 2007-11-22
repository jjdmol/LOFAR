//#  FirMipmap: Tryout to build a FIR filter on a GPU.
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
//
// This program demonstrates the function of a series of FIR filters.
// The shape of the FIR filter is sin(x)/x and is stored in 4096 samples.
// Each FIR filter has 16 latches. So we use (4096/16=)256 simultaneous channels
// to filter the captured signal.
// The captured signal is simulated with a single pulse, so we should see the
// shape of the Fir filters as the result of the calculations.
//
// The program uses only 1 frequency band in this demo but the demonstrated
// principle should be done in 200 fold in real life.
//
#include <iostream>
#include <math.h>			// atoi
#include <string.h>			// memset
#include <sys/time.h>		// timer
#include "GPUEnvironment.h"
#include "Stream.h"

// Compiler flags to see some intermediate results
// Setting the first flag to 1 disables all other flags.
#define	CALC_TIMES		 1						// show relative performance
#define	SHOW_FIRFILTER	(0 >> CALC_TIMES)		// show elements of FIRfilter
#define	SHOW_DATAPOINTS	(0 >> CALC_TIMES)		// show generated datapoints
#define	SHOW_LATCHES	(1 >> CALC_TIMES)		// show contents of latches
#define	CHECK_RESULTS	(1 >> CALC_TIMES)		// compare CPU with GPU results

using namespace std;

#define	NR_BANDS			200		// should be 200..300
#define	NR_CHANNELS			256		// nr of fir filters per freq. band
#define	NR_LATCHES			16		// nr of latches in each fir filter
#define	NR_POLAR			2		// not used yet in the program!

#define	NR_RUNS				16		// to see the result of all latches

// Scaling factors for mapping the latches on the GPU.
// The 16 latches are mapped as 2x2 pixels of 4 floats (RGBA)
#define	PIXEL_SZ			2
#define PIXEL_FACTOR		16		// 2 x 2 x 4 = PIX_SZ x PIX_SZ x flt4
#define LATCH_SCALE			(PIXEL_FACTOR / PIXEL_SZ)

// define structure that contains all 16 latches of 1 FIR filter
typedef struct _floatPix {
	float4	p[PIXEL_SZ][PIXEL_SZ];
} floatPix;

// Global variables
static int	gLatchIdx = -1;		// used in CPU vers to select the latch column
static int	gPeak     = 1;		// used by datagenerator
static int	gRunNr    = 0;		// several times used as offset in array

// CPU version datastructures
float	cpuLatch  [NR_BANDS][NR_CHANNELS][NR_LATCHES];	// for reference of data
float	cpuResult [NR_BANDS][NR_CHANNELS][NR_RUNS];		// results of all runs

// Shared structures
// Note: the firfilter is for all bands equal
float	firFilter [NR_CHANNELS][NR_LATCHES];	// holds coeff of filters
float	datapoint [NR_BANDS][NR_CHANNELS];		// dataset to feed all filters

// GPU version datastructures
// Note: since all latches are in one floatPix the arrays don't have
// a NR_LATCHES dimension anymore.
floatPix	peekArr   [NR_BANDS][NR_CHANNELS];	// result array
floatPix	gpuLatch  [NR_BANDS][NR_CHANNELS];	// latch data
float		gpuResult [NR_BANDS][NR_CHANNELS];	// result array

//------------------------- C version routines ------------------------- 
//
// init firFilter
//
// Fill the filter elements with the function sin(x)/x
//
void initSincFilter (float*		fp, int		nrElems)
{
	int		bound  = nrElems / 2;
	int		nrZero = 5;
	int		Ts = ((bound / nrZero) / 10) * 10;

	printf ("Generating sincFilter of %d elements with %d zero's\n", 
														nrElems, nrZero * 2);

	for (int x = -bound; x < bound; x++) {
		if (x)
			fp[x + bound] = sin(M_PI * x / Ts) / (M_PI * x / Ts);
		else
			fp[x + bound] = 1.0;
//		printf ("%4d: %10.8f\n", x + bound, fp[x + bound]);
	}
	
#if SHOW_FIRFILTER
	printf ("Coeff of Firfilter in channel 0\n");
	for (int l = 0; l < NR_LATCHES; l++)
		printf ("%4d: %10.8f\n", l, firFilter[l][0]);
#endif
		
}


//
// loadFilters
//
// Load the new datapoint into the right latch registers
// Uses the global gLatchIdx to address the right latch column
//
void loadFilters(void)
{
	gLatchIdx = (gLatchIdx + 1) % NR_LATCHES;
	for (int b = 0; b < NR_BANDS; b++) {
		for (int c = 0; c < NR_CHANNELS; c++) {
			cpuLatch[b][c][gLatchIdx] = datapoint[b][c];
		}
	}

#if SHOW_LATCHES
	printf ("FIRlatches at channel 0\n");
	for (int l = 0; l < NR_LATCHES; l++)
		printf ("%3.1f|", cpuLatch[0][0][l]);
	printf("\n");
#endif
}


//
// calcFilters
//
// Multiply (and add) the datapoints with the latches
//
void calcFilters(void)
{
	for (int b = 0; b < NR_BANDS; b++) {
		for (int c = 0; c < NR_CHANNELS; c++) {
			cpuResult[b][c][gRunNr] = 0;
			for (int l = 0; l < NR_LATCHES; l++) {
				cpuResult[b][c][gRunNr] += (firFilter[c][(l+gLatchIdx)%NR_LATCHES] * 
											cpuLatch[b][c][l]);
			}
		}
	}
}


//
// showFilterOutput
//
// Show the contents of the calculated filter results.
// NOTE: uses the global gRunNr to show the right results.
//
void showFilterOutput(void)
{
	printf("-----\n");
	for (int c = 0; c < NR_CHANNELS; c++) {
		if (cpuResult[0][c][gRunNr] != 0.0)
			printf ("%3d: %10f\n", c, cpuResult[0][c][gRunNr]);
	}
}

//------------------------- 'shared' routines ------------------------- 
//
// getDataset
//
// simulates a pulse every 256 samples
//
void getDataset (void)
{
	for (int b = 0; b < NR_BANDS; b++) {
		for (int c = 0; c < NR_CHANNELS; c++) {
			if (gPeak)
				datapoint[b][c] = 1.0;
			else
				datapoint[b][c] = 0.0;
		}
	}

	gPeak = 0;

#if SHOW_DATAPOINTS
	printf ("Datapoints in first 10 channels of band 0\n");
	for (int c = 0; c < 10; c++)
		printf ("%3.1f ", datapoint[0][c]);
	printf ("\n");
#endif
}


//------------------------- GPU version routines ------------------------- 
//
// mapDataset
//
// Maps datapoints into gpuLatch texture
//
void mapDataset(void)
{
	for (int b = 0; b < NR_BANDS; b++) {
		for (int c = 0; c < NR_CHANNELS; c++) {
			memmove(&gpuLatch[b][c].p[0][0].g, &gpuLatch[b][c].p[0][0].r,
										(NR_LATCHES - 1) * sizeof(float));
			gpuLatch[b][c].p[0][0].r = datapoint[b][c];
		}
	}
	
#if SHOW_LATCHES
	printf ("Data in first 5 latches of the first n channels\n");
	for (int c = 0; c < 3; c++) {
		float*	fp = &gpuLatch[0][c].p[0][0].r;
		printf ("gpudata channel %d: %3.1f|%3.1f|%3.1f|%3.1f|%3.1f\n", c, fp[0], fp[1], fp[2], fp[3], fp[4]);
	}
#endif			
}


// 
// Show floatPix structure
//
// prints the contents of 1 floatpix structure
//
void showFloatPix (floatPix*	pix) {
	printf (" RG      RG\n");
	printf (" BA      BA\n");
	printf ("[%5.3f , %5.3f]      [%5.3f , %5.3f]\n", pix->p[0][0].r, pix->p[0][0].g, 
													  pix->p[1][0].r, pix->p[1][0].g);
	printf ("[%5.3f , %5.3f]      [%5.3f , %5.3f]\n", pix->p[0][0].b, pix->p[0][0].a, 
													  pix->p[1][0].b, pix->p[1][0].a);
	printf ("\n");
	printf ("[%5.3f , %5.3f]      [%5.3f , %5.3f]\n", pix->p[0][1].r, pix->p[0][1].g, 
													  pix->p[1][1].r, pix->p[1][1].g);
	printf ("[%5.3f , %5.3f]      [%5.3f , %5.3f]\n", pix->p[0][1].b, pix->p[0][1].a, 
													  pix->p[1][1].b, pix->p[1][1].a);
}


//
// MAIN
//
int main (int	argc, char *argv[]) {
	double			gpuTimer, cpuTimer;
	struct timeval	tstart, tstop;

	// Create a GPU environment for fragment programs
	GPUEnvironment	GPUEnv (string(""), string("fp30"), 2048);
	
	// Check invocation
	if (argc !=2) {
		printf ("Syntax: %s filterProg\n", argv[0]);
		return (0);
	}

	// Create the Cg programs
	GPUEnv.makeCurrentGPUEnv();
	CGprogram	FirProg = GPUEnv.compileProgram(GPUEnvironment::fragmentProg,
												argv[1], "fragmentMain");
	if (!FirProg) {
		return (0);
	}

	// Show info about GPU environment
	GPUEnv.info();

	// setup testdata and clear result
	initSincFilter((float*)&firFilter[0][0], sizeof(firFilter) / sizeof(float));

	// clear latches from CPU version
	memset(&cpuLatch[0][0][0], 0, NR_BANDS*NR_CHANNELS*NR_LATCHES * sizeof(float));

	//
	// Execute the CPU version
	//
	gettimeofday(&tstart, NULL);
	for (gRunNr = 0; gRunNr < NR_RUNS; gRunNr++) {
		getDataset();				// generated for now
		loadFilters();				// map new data into latches
		calcFilters();				// multiply and add
//		showFilterOutput();
	}
	gettimeofday(&tstop, NULL);
	cpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;

	//
	// Prepare GPU version
	//
	// Create a stream for reading the result.
	Stream	peekStream(NR_BANDS*NR_LATCHES/PIXEL_FACTOR, NR_CHANNELS*PIXEL_SZ, 
													Stream::Float4, peekArr);

	// create input matrix and streams
	Stream	filterStream(NR_BANDS*NR_LATCHES/PIXEL_FACTOR*PIXEL_SZ, 
							NR_CHANNELS*PIXEL_SZ, Stream::Float4, firFilter);
	Stream	dataStream  (NR_BANDS*NR_LATCHES/PIXEL_FACTOR*PIXEL_SZ, 
							NR_CHANNELS*PIXEL_SZ, Stream::Float4, gpuLatch);
	Stream	resultStream(NR_BANDS*NR_LATCHES/PIXEL_FACTOR, 
							NR_CHANNELS, Stream::Float1, gpuResult);

	// Tell card which program to use
	GPUEnv.useProgram (FirProg);

	filterStream.writeData();			// load filter coeff on card

	//
	// Execute GPU version
	//
	gPeak = 1;							// reset dataset.
	gettimeofday(&tstart, NULL);
	for (gRunNr = 0; gRunNr < NR_RUNS; gRunNr++) {
		getDataset();					// get new set of samples
		mapDataset();					// map samples on GPU dataset
		dataStream.writeData();			// move data to graphics card

		// tell kernel the order of usage in fragmentprogram
		clearStreamStack();
		dataStream.use();
		filterStream.use();

		// calculate FIR matrix
		GPUEnv.executeFragmentSquare(NR_BANDS*NR_LATCHES/PIXEL_FACTOR*PIXEL_SZ/2, 
													NR_CHANNELS*PIXEL_SZ/2);	
		resultStream.readScreen();
//		printf ("gpuResult channel 0-4: %10.8f|%10.8f|%10.8f|%10.8f|%10.8f\n", gpuResult[0][0], gpuResult[0][1], gpuResult[0][2], gpuResult[0][3], gpuResult[0][4]);

#if CHECK_RESULTS
		int	nrErrors = 0;
		for (int c = 0; c < NR_CHANNELS; c++) {
			if (gpuResult[c] != cpuResult[c][gRunNr])
				nrErrors++;
		}
		printf ("channel %d: %d errors\n", 0, nrErrors);
#endif
	}
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;

#if CALC_TIMES
	printf("Elapsed time: %f seconds\n", cpuTimer);
	printf("Elapsed time: %f seconds\n", gpuTimer);
	printf("Factor = %3.1f\n", gpuTimer / cpuTimer);
#endif	

}
