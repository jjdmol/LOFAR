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

//# Example how to make a GPU program

#include <iostream>
#include <stdlib.h>			// atoi
#include <string.h>			// memset
#include "GPUEnvironment.h"
#include "Stream.h"

using namespace std;

#define	TEXTURE_SIZE		1024
#define	TEXTURE_SQUARE		(TEXTURE_SIZE * TEXTURE_SIZE)
#define	SIGNAL_DEPTH		2

float4			inputArr  [TEXTURE_SQUARE];		// input matrix
float4			resultArr [TEXTURE_SQUARE];		// result matrix
float4			real	  [TEXTURE_SIZE][SIGNAL_DEPTH];	// real	


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
	}
}

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
	}
}

//
// MAIN
//
int main (int	argc, char *argv[]) {

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

	// setup testdata and clear result
	initInputArrays();

	// Create a stream for reading the result.
	Stream	resultStream(TEXTURE_SIZE, TEXTURE_SIZE, Stream::Float4, resultArr);

	// create input matrix and streams
	Stream	realStream (TEXTURE_SIZE, SIGNAL_DEPTH, Stream::Float4, real);

	// Tell card which program to use
	GPUEnv.useProgram (CorrProg);

	for (int l = 0; l < loop; l++) {
		inputStream.writeData();				// clear result matrix
		// or glClear(GL_COLOR_BUFFER_BIT);
		for (int r = 0; r < nrRuns; r++) {
			// load signal samples
			realStream.writeData();
	
			// tell kernel the order of usage in fragmentprogram
			clearStreamStack();
			realStream.use();
			inputStream.use();
	
			// calculate corr. matrix
			GPUEnv.executeFragmentTriangle(TEXTURE_SIZE, TEXTURE_SIZE);	
			// or
			// GPUEnv.executeFragmentSquare(TEXTURE_SIZE, TEXTURE_SIZE);	
			inputStream.copyScreen();			// copy result to texture
		}
		resultStream.readScreen();
	}
}
