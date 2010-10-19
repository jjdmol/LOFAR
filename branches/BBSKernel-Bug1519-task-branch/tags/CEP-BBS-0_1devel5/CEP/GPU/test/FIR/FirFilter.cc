//#  FirFilter: Tryout to build a FIR filter on a GPU.
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
// to filter the captured signal. (downsampling)
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
#define	SHOW_LATCHES	(0 >> CALC_TIMES)		// show contents of latches
#define	CHECK_RESULTS	(1 >> CALC_TIMES)		// compare CPU with GPU results

using namespace std;

#define	NR_BANDS			100		// should be 200..300
#define	NR_CHANNELS			256		// nr of fir filters per freq. band
#define	NR_LATCHES			16		// nr of latches in each fir filter
#define	NR_POLAR			2		// not used yet in the program!

#define	NR_RUNS				16		// to see the result of all latches

// Global variables
static int	gLatchIdx = -1;			// used in CPU vers to select latch column
static int	gPeak     = 1;			// used by datagenerator
static int	gRunNr    = 0;			// several times used as offset in array

// CPU version datastructures
float	cpuLatch  [NR_BANDS][NR_CHANNELS][NR_LATCHES];	// for reference of data
float	cpuResult [NR_BANDS][NR_CHANNELS][NR_RUNS];		// results of all runs

// Shared structures
// Note: the firfilter is for all bands equal
float	firFilter [NR_CHANNELS][NR_LATCHES];	// holds coeff of filters
float	datapoint [NR_BANDS][NR_CHANNELS];		// dataset to feed all filters

// GPU version datastructures
float	gpuLatch  [NR_BANDS][NR_CHANNELS][NR_LATCHES];	// fir latches on GPU
float	gpuResult [NR_CHANNELS];				// result of a single run/band

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
		int Xn = (x+bound) % NR_CHANNELS;
		int Yn = (x+bound) / NR_CHANNELS; 
		int Idx = Xn * NR_LATCHES + Yn;
		if (x)
			fp[Idx] = sin(M_PI * x / Ts) / (M_PI * x / Ts);
		else
			fp[Idx] = 1.0;
//		printf ("%04d: %10.8f\n", Idx, fp[Idx]);
	}
	
#if SHOW_FIRFILTER
	printf ("Coeff of Firfilter 0\n");
	for (int l = 0; l < NR_LATCHES; l++)
		printf ("%4d: %10.8f\n", l, firFilter[0][l]);
#endif
		
}


//
// getDataset
//
// Simulates a pulse of 256 samples
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


//
// mapDataset
//
// Maps the datapoints into the gpuLatches by moving up the data in the
// latches 1 place and storing the new data samples in latch 0 of the channels.
//
void mapDataset(void)
{
	for (int b = 0; b < NR_BANDS; b++) {
		for (int c = 0; c < NR_CHANNELS; c++) {
			memmove(&gpuLatch[b][c][1], &gpuLatch[b][c][0], 
										(NR_LATCHES - 1) * sizeof(float));
			gpuLatch[b][c][0] = datapoint[b][c];
		}

#if SHOW_LATCHES
	printf ("Data in first 5 latches of the first n channels\n");
	for (int c = 0; c < 3; c++) {
		printf ("gpudata channel %d: %3.1f|%3.1f|%3.1f|%3.1f|%3.1f\n", c, gpuLatch[0][c][0], gpuLatch[0][c][1], gpuLatch[0][c][2], gpuLatch[0][c][3], gpuLatch[0][c][4]);
	}
#endif			
	}
}


//
// loadFilters
//
// Load the new datapoint into the right latch registers
// Uses the global gLatchIdx to address the rigth latch column.
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
// Multiply and add the latches with the filter coeff.
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


//
// MAIN
//
int main (int	argc, char *argv[]) {
	double			gpuTimer, cpuTimer;
	struct timeval	tstart, tstop;

	// Create a GPU environment for fragment programs
	GPUEnvironment	GPUEnv (string(""), string("fp30"), 2048);
	
	// Check invocation
	if (argc !=3) {
		printf ("Syntax: %s multProg addProg\n", argv[0]);
		return (0);
	}

	// Create the Cg programs
	GPUEnv.makeCurrentGPUEnv();
	CGprogram	FirProg = GPUEnv.compileProgram(GPUEnvironment::fragmentProg,
												argv[1], "fragmentMain");
	CGprogram	AddProg = GPUEnv.compileProgram(GPUEnvironment::fragmentProg,
												argv[2], "fragmentMain");
	if (!FirProg || !AddProg) {
		return (0);
	}

	// Show info about GPU environment
	GPUEnv.info();

	// setup testdata and clear result
	initSincFilter((float*)&firFilter[0][0], sizeof(firFilter) / sizeof(float));
	memset(&cpuLatch[0][0][0], 0, NR_BANDS*NR_CHANNELS*NR_LATCHES * sizeof(float));
	memset(&gpuLatch[0][0][0], 0, NR_BANDS*NR_CHANNELS*NR_LATCHES * sizeof(float));

	//
	// Execute the CPU version
	//
	gettimeofday(&tstart, NULL);
	for (gRunNr = 0; gRunNr < NR_RUNS; gRunNr++) {
		getDataset();				// generate new dataset
		loadFilters();				// map new data into latches
		calcFilters();				// multiply and add
//		showFilterOutput();
	}
	gettimeofday(&tstop, NULL);
	cpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;

	//
	// Prepare the GPU version
	//
	// create a stream for reading the result.
	Stream	resultStream(1, NR_CHANNELS, Stream::Float1, gpuResult);

	// create stream for coeff. and latches.
	// Note: they are declared as float4!
	Stream	latchStream(NR_BANDS*NR_LATCHES/4, NR_CHANNELS, Stream::Float4, gpuLatch);
	Stream	filterStream(NR_LATCHES/4, NR_CHANNELS, Stream::Float4, firFilter);

	// load filter coeff on the card
	filterStream.writeData();

	// tell kernel the order of usage in fragmentprogram
	clearStreamStack();
	latchStream.use();
	filterStream.use();

	//
	// Execute the GPU version
	//
	gPeak = 1;		// reset datagenerator
	gettimeofday(&tstart, NULL);
	for (gRunNr = 0; gRunNr < NR_RUNS; gRunNr++) {
		getDataset();						// get new set of samples
		mapDataset();						// map samples on GPU dataset
		latchStream.writeData();			// move data to graphics card

		// calculate FIR matrix
		GPUEnv.useProgram (FirProg);
		GPUEnv.executeFragmentSquare(NR_BANDS*NR_LATCHES/4, NR_CHANNELS);	

		// Let the GPU do the add.
		latchStream.copyScreen();
		GPUEnv.useProgram (AddProg);
		for (int b = 0; b < NR_BANDS; b++) {
			GPUEnv.executeFragmentVLine(b * (NR_LATCHES/4));
			resultStream.readScreen();		// !! should be shifted to next band

#if CHECK_RESULTS
			int	nrErrors = 0;
			for (int c = 0; c < NR_CHANNELS; c++) {
				if (gpuResult[c] != cpuResult[b][c][gRunNr])
					nrErrors++;
			}
			printf ("band %d, channel %d: %d errors\n", b, 0, nrErrors);
#endif
		} // b
	}	// gRunNr
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;

#if CALC_TIMES
	printf("Elapsed time CPU: %f seconds\n", cpuTimer);
	printf("Elapsed time GPU: %f seconds\n", gpuTimer);
	printf("Factor = %3.1f\n", gpuTimer / cpuTimer);
#endif
	
}
