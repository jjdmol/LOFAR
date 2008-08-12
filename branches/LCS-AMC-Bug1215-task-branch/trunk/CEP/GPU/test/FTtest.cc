#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include "Stream.h"
#include "PixelBuffer.h"
#include "GPU.h"

using namespace std;

#define NFACTORIAL		12
#define PI  			3.141592653589793
#define PI2 			6.283185307179586
#define NR_STATIONS		100
#define NR_INTERVALS	512
#define	NR_FREQS		512

// Define the array holding exp(i(ul+vm+wn)) per station per time.
float4 expx[NR_FREQS];
float4 expy[NR_INTERVALS];
float4 result[NR_FREQS][NR_INTERVALS];

int main(int argc , char *argv[])
{
	struct timeval	tstart, tstop;
	double			gpuTimer;
	PixelBuffer		PBuffer(4);				// allocate bitplanes

	// Check invocation
	if (argc !=2) {
		printf ("Syntax: %s fragmentfile\n", argv[0]);
		return (0);
	}

	// Create the GL environment
	if (!setupGlEnvironment(argv[1])) {
		return (0);
	}

//	cout << "Nr of stations    : " << NR_STATIONS << endl;
	cout << "Nr of frequencies : " << NR_FREQS << endl;
	cout << "Nr of intervals   : " << NR_INTERVALS << endl;

	for (int i = 0; i < NR_FREQS; i++) {
		expx[i].r = (rand() % 1000000) / 1000000.0;
		expx[i].g = (rand() % 1000000) / 1000000.0;
		expx[i].b = (rand() % 1000000) / 1000000.0;
		expx[i].a = (rand() % 1000000) / 1000000.0;
	}
	for (int i = 0; i < NR_INTERVALS; i++) {
		expy[i].r = (rand() % 1000000) / 1000000.0;
		expy[i].g = (rand() % 1000000) / 1000000.0;
		expy[i].b = (rand() % 1000000) / 1000000.0;
		expy[i].a = (rand() % 1000000) / 1000000.0;
	}

	Stream	argxStream(NR_FREQS,     1, Stream::Float4, expx);
	Stream	argyStream(NR_INTERVALS, 1, Stream::Float4, expy);
	Stream	resStream (NR_FREQS, NR_INTERVALS, Stream::Float4, result);

	printf ("Calculating %d e's on GPU\n", NR_INTERVALS * NR_FREQS * 4);
	gettimeofday(&tstart, NULL);
	argxStream.writeData();				// load x data
	argyStream.writeData();				// load y data
	clearStreamStack();					// tell kernel order of streams
	argxStream.use();
	argyStream.use();
	executeFragmentSquare(NR_FREQS, NR_INTERVALS);	// execute the program
	resStream.readScreen();				// retrieve the result
	gettimeofday(&tstop, NULL);
	gpuTimer = tstop.tv_sec-tstart.tv_sec+(tstop.tv_usec-tstart.tv_usec)/1000000.0;
	printf("Elapsed time: %f seconds\n", gpuTimer);
	for (int x =0; x < 5; x++) {
		for (int y =0; y < 5; y++) {
			printf ("%2d,%2d: %f,%f =  %f\n", x, y, 
									expx[x].r, expy[y].r, result[x][y].r);
		}
	}

	int count = 0;
	for (int l = 0; l < NR_STATIONS; l++) {
		for (int r = 0; r < l; r++) {
			count++;
		}
	}
	cout << count << endl;

	return (0);
}
