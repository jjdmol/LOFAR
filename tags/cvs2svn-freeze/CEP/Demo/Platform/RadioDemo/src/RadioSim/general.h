#ifndef __GENERAL_H
#define __GENERAL_H

#include <stdlib.h>
#include <complex>

#ifdef MPI_
#include "mpi.h"
#endif

#define MPI_OPT
#include "firewalls.h"

// LAGS
const int LAGS=1;

// Number of stations
const int STATIONS = 2;

// number of dataprocessors
const int DATAPROCESSORS = 1;


// Number of antennas/ffts
const int ELEMENTS = 1;     
const int ANTSAMPLES = 256;//128; 
const int BEAMS = 1;		//max=64 (power of 4)
const int FREQS = ANTSAMPLES;		//64
const int CORRFREQS = FREQS;
const int FREQBANDWIDTH = CORRFREQS;
const int FREQBANDS  = FREQS/CORRFREQS;
const int FCORR = FREQBANDS;  // 64/16=4
const int POLS = 1;


// Node of the controller
//const int CONTROLLER_NODE = 0;


// Antenna to print FFT, BEAM, etc
const int FFT_GRAPH = 1; // id of fft to plot
const int BEAM_GRAPH = 1; // id of fft to plot
const int FREQ_GRAPH = 1; // frequentie of Beams to plot


// Input of antenna

const float INPUT_FREQ=20e6;        // Signal at frequency 5 Mhz
const float INPUT_CYCLE=1./5e6;    // 200 ns
const float INPUT_AZIMUTH=0.0;      // Azimuth of frequency 
const float INPUT_ELEVATION=90.0;   // Elevation 

// Samples [ 0.. 20MHz ] -> (1/20e9) = 50 ns

const float ANTSAMPLETIME = 2.51e-8;    // Sample of 50 ns 
const float LIGHT_SPEED = 299792480.;     // m/sec
const float PI=3.14159265;


//enum procMode{zeroes,ones,infile,skip,process};

typedef std::complex < float >  DataBufferType;

//class TH_MPI;


/* #ifdef NOMPI_ // Do NOT use MPI */
/* #define TRANSPORTER TH_Corba // to MPI or  not to MPI .. */
/* #else // use MPI */
/* #define TRANSPORTER TH_MPI // to MPI or  not to MPI .. */
/* //#include "TH_MPI.h" // include after the typdef TRANSPORTER */
/* #endif // MPI */

// define action for different levels later:
//    debug, monitor,error

#define TRACER(level,stream) if(0) cout<< stream << endl

//#define _Exception CException

#endif
