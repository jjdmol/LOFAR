#define STATIONS (2*100)                        // 100 stations, each 2 polarisations
#define CORRELATIONS (STATIONS*(STATIONS))
#define INTEGRATIONFACTOR 10000                 // corresponds to 1 sec, 10 kHz dump
#define FREQUENCYCHANNELS 50                   // number of frequency channels to be processed on this node
#define TIME 1                                  // duration of the run (seconds of input data)

#include "myComplex.h"
#include <stdlib.h>
#include "mpi.h"

main (int argc, char *argv[]) {

  MPI_Init(&argc, &argv);
  int myrank;
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

  myComplex32 Correlation[CORRELATIONS];
  myComplex8  StationData[STATIONS];

  if (myrank == 0) {
    // fill the station data array with random data
    for (int i=0; i<STATIONS; i++) {
      StationData[i] = myComplex8((int)(127.0*random()/RAND_MAX+1.0),
				  (int)(127.0*random()/RAND_MAX+1.0));
    }
  }
  
  for (int time=0; time < TIME; time++) {

    for (int f=0; f<FREQUENCYCHANNELS; f++) {
      if (myrank == 1) {
	// reset the integrator
	for (int i=0; i<CORRELATIONS; i++) Correlation[i]=0;
      }

      for (int integrator=0; integrator < INTEGRATIONFACTOR; integrator++) { 
	
	if (myrank == 0) {
	  // swap two values in the input data
	  int a = (int)(1. * STATIONS * random()/RAND_MAX);
	  int b = (int)(1. * STATIONS * random()/RAND_MAX);
	  myComplex8 tmp = StationData[a];
	  StationData[a] = StationData[b];
	  StationData[b] = tmp;
	  MPI_Send(StationData, STATIONS*2, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
	}
	
	if (myrank == 1) {
	  MPI_Status status;
	  MPI_Recv(StationData, STATIONS*2, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
	  for (int stationA=0; stationA<STATIONS; stationA++) {
	    for (int stationB=0; stationB <= stationA; stationB++) {
	      Correlation[stationB*STATIONS+stationA].cmac(StationData[stationA],StationData[stationB]);
	    }
	  }
	}
      }
      if (myrank == 1) MPI_Send(Correlation, CORRELATIONS*2, MPI_BYTE, 2, 1, MPI_COMM_WORLD);
      if (myrank == 2) {
	MPI_Status status;
	MPI_Recv(Correlation, CORRELATIONS*2, MPI_BYTE, 1, 1, MPI_COMM_WORLD, &status);
      }
    }
    if (myrank == 2) cout << time << " " << Correlation[1] << endl;

  }
  MPI_Finalize();
}
