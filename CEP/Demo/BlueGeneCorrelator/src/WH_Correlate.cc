//#  WH_Correlate.cc: Correlator WorkHolder for a BG corerlator application
//#
//#  Copyright (C) 2002-2004
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

#include <stdio.h>         // for sprintf

// General LOFAR includes
#include <Common/Debug.h>
#include <Common/KeyValueMap.h>

// Application Specific includes
#include <WH_Correlate.h>
#include <BlueGeneCorrelator/definitions.h>

#ifdef __BLRTS__
MPI_Datatype my_complex;
#endif

using namespace LOFAR;

WH_Correlate::WH_Correlate(const string& name,
			   unsigned int nin)
  : WorkHolder(nin, nin, name, "WH_Correlate"),
    myrank(0),
    mysize(0)
{

  char str[8];

  for (unsigned int i = 0; i < nin; i++) {
    
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_CorrCube(string("in_")+str));
    getDataManager().addOutDataHolder(i, 
				      new DH_Vis(string("out_")+str));
  }
}

WH_Correlate::~WH_Correlate() {
}

WorkHolder* WH_Correlate::construct(const string& name, unsigned int channels) {
  return new WH_Correlate(name, channels);
}

WH_Correlate* WH_Correlate::make(const string& name) {
  return new WH_Correlate(name, getDataManager().getInputs());
}

void WH_Correlate::preprocess() {
#ifdef __BLRTS__

  
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &mysize);

  // Define global MPI complex float datatype
  MPI_Type_contiguous(2, MPI_FLOAT, &my_complex);
  MPI_Type_commit(&my_complex);
  
#endif
}

void WH_Correlate::process() {

  if (myrank == 0) {

    memcpy(sig_buf, 
	   ((DH_CorrCube*)getDataManager().getInHolder(0))->getBuffer(),
	   NSTATIONS*NCHANNELS*NSAMPLES*sizeof(DH_CorrCube::BufferType));

    WH_Correlate::master();

    memcpy(((DH_Vis*)getDataManager().getOutHolder(0))->getBuffer(),
	   sig_buf,
	   NSTATIONS*NSTATIONS*NCHANNELS*sizeof(DH_Vis::BufferType));

  } else {

    WH_Correlate::slave(myrank);

  }
}

void WH_Correlate::dump() {
}

void WH_Correlate::correlator_core(complex<float> signal[NSTATIONS][NSAMPLES],
				   complex<float> corr[NSTATIONS][NSTATIONS]) {
  
  /* This does the cross and autocorrelation on the lower half of the matrix */
  /* The upper half is not calculated, but is simply the complex conjugate   */
  /* the lower half. This should be implemented as a post correlation        */
  /* procedure later on, but is for now omitted. -- CB                       */

  int x, y;
  for (int time = 0; time < NSAMPLES; time++) {
    for (x = 0; x < NSTATIONS; x++) {
      for (y = 0; y <= x; y++) {
	corr[x][y] += complex<float> (
                      signal[x][time].real() * signal[y][time].real() -  // real 
		      signal[x][time].imag() * signal[y][time].imag(),
				      
		      signal[x][time].real() * signal[y][time].imag() +  // imag
		      signal[x][time].imag() * signal[y][time].real()
		      );
      }
    }
  }
}

void WH_Correlate::correlator_core_unrolled(complex<float> s[NSTATIONS][NSAMPLES],
					    complex<float> c[NSTATIONS][NSTATIONS]) {

  /* This does the cross and autocorrelation on the lower half of the matrix */
  /* The upper half is not calculated, but is simply the complex conjugate   */
  /* the lower half. This should be implemented as a post correlation        */
  /* procedure later on, but is for now omitted. -- CB                       */

  int loop = 5;
  int x, y;

  for ( int time = 0; time < NSAMPLES; time++) {
    for ( x = 0; (x+loop) < NSTATIONS; x += loop ) {
      for ( y = 0; (y+loop) <= x; y += loop ) {

	c[x  ][y  ] += complex<float> (
		     s[x  ][time].real() * s[y  ][time].real() - s[x  ][time].imag() * s[y  ][time].imag(),
		     s[x  ][time].real() * s[y  ][time].imag() + s[x  ][time].imag() * s[y  ][time].real()
		     );
      
	c[x  ][y+1] += complex<float> (
		     s[x  ][time].real() * s[y+1][time].real() - s[x  ][time].imag() * s[y+1][time].imag(),
		     s[x  ][time].real() * s[y+1][time].imag() + s[x  ][time].imag() * s[y+1][time].real()
		     );
      
	c[x  ][y+2] += complex<float> (
		     s[x  ][time].real() * s[y+2][time].real() - s[x  ][time].imag() * s[y+2][time].imag(),
		     s[x  ][time].real() * s[y+2][time].imag() + s[x  ][time].imag() * s[y+2][time].real()
		     );
      
	c[x  ][y+3] += complex<float> (
		     s[x  ][time].real() * s[y+3][time].real() - s[x  ][time].imag() * s[y+3][time].imag(),
		     s[x  ][time].real() * s[y+3][time].imag() + s[x  ][time].imag() * s[y+3][time].real()
		     );
      
	c[x  ][y+4] += complex<float> (
		     s[x  ][time].real() * s[y+4][time].real() - s[x  ][time].imag() * s[y+4][time].imag(),
		     s[x  ][time].real() * s[y+4][time].imag() + s[x  ][time].imag() * s[y+4][time].real() 
		     );
            
	c[x+1][y  ] += complex<float> (
		     s[x+1][time].real() * s[y  ][time].real() - s[x+1][time].imag() * s[y  ][time].imag(),
		     s[x+1][time].real() * s[y  ][time].imag() + s[x+1][time].imag() * s[y  ][time].real()
		     );
      
	c[x+1][y+1] += complex<float> (
		     s[x+1][time].real() * s[y+1][time].real() - s[x+1][time].imag() * s[y+1][time].imag(),
		     s[x+1][time].real() * s[y+1][time].imag() + s[x+1][time].imag() * s[y+1][time].real()
		     );
      
	c[x+1][y+2] += complex<float> (
		     s[x+1][time].real() * s[y+2][time].real() - s[x+1][time].imag() * s[y+2][time].imag(),
		     s[x+1][time].real() * s[y+2][time].imag() + s[x+1][time].imag() * s[y+2][time].real()
		     );
      
	c[x+1][y+3] += complex<float> (
		     s[x+1][time].real() * s[y+3][time].real() - s[x+1][time].imag() * s[y+3][time].imag(),
		     s[x+1][time].real() * s[y+3][time].imag() + s[x+1][time].imag() * s[y+3][time].real() 
		     );
      
	c[x+1][y+4] += complex<float> (
		     s[x+1][time].real() * s[y+4][time].real() - s[x+1][time].imag() * s[y+4][time].imag(),
		     s[x+1][time].real() * s[y+4][time].imag() + s[x+1][time].imag() * s[y+4][time].real()
		     );
            
	c[x+2][y  ] += complex<float> (
		     s[x+2][time].real() * s[y  ][time].real() - s[x+2][time].imag() * s[y  ][time].imag(),
		     s[x+2][time].real() * s[y  ][time].imag() + s[x+2][time].imag() * s[y  ][time].real()
		     );
      
      c[x+2][y+1] += complex<float> (
		     s[x+2][time].real() * s[y+1][time].real() - s[x+2][time].imag() * s[y+1][time].imag(),
		     s[x+2][time].real() * s[y+1][time].imag() + s[x+2][time].imag() * s[y+1][time].real()
		     );
      
      c[x+2][y+2] += complex<float> (
		     s[x+2][time].real() * s[y+2][time].real() - s[x+2][time].imag() * s[y+2][time].imag(),
		     s[x+2][time].real() * s[y+2][time].imag() + s[x+2][time].imag() * s[y+2][time].real()
		     );
      
      c[x+2][y+3] += complex<float> (
		     s[x+2][time].real() * s[y+3][time].real() - s[x+2][time].imag() * s[y+3][time].imag(),
		     s[x+2][time].real() * s[y+3][time].imag() + s[x+2][time].imag() * s[y+3][time].real()
		     );
      
      c[x+2][y+4] += complex<float> (
		     s[x+2][time].real() * s[y+4][time].real() - s[x+2][time].imag() * s[y+4][time].imag(),
		     s[x+2][time].real() * s[y+4][time].imag() + s[x+2][time].imag() * s[y+4][time].real()
		     );
      
      
      c[x+3][y  ] += complex<float> (
		     s[x+3][time].real() * s[y  ][time].real() - s[x+3][time].imag() * s[y  ][time].imag(),
		     s[x+3][time].real() * s[y  ][time].imag() + s[x+3][time].imag() * s[y  ][time].real()
		     );
      
      c[x+3][y+1] += complex<float> (
		     s[x+3][time].real() * s[y+1][time].real() - s[x+3][time].imag() * s[y+1][time].imag(),
		     s[x+3][time].real() * s[y+1][time].imag() + s[x+3][time].imag() * s[y+1][time].real()
		     );
      
      c[x+3][y+2] += complex<float> (
		     s[x+3][time].real() * s[y+2][time].real() - s[x+3][time].imag() * s[y+2][time].imag(),
		     s[x+3][time].real() * s[y+2][time].imag() + s[x+3][time].imag() * s[y+2][time].real()
		     );
      
      c[x+3][y+3] += complex<float> (
		     s[x+3][time].real() * s[y+3][time].real() - s[x+3][time].imag() * s[y+3][time].imag(),
		     s[x+3][time].real() * s[y+3][time].imag() + s[x+3][time].imag() * s[y+3][time].real()
		     );
      
      c[x+3][y+4] += complex<float> (
		     s[x+3][time].real() * s[y+4][time].real() - s[x+3][time].imag() * s[y+4][time].imag(),
		     s[x+3][time].real() * s[y+4][time].imag() + s[x+3][time].imag() * s[y+4][time].real()
		     );
      
      
      c[x+4][y  ] += complex<float> (
		     s[x+4][time].real() * s[y  ][time].real() - s[x+4][time].imag() * s[y  ][time].imag(),
		     s[x+4][time].real() * s[y  ][time].imag() + s[x+4][time].imag() * s[y  ][time].real()
		     );
      
      c[x+4][y+1] += complex<float> (
		     s[x+4][time].real() * s[y+1][time].real() - s[x+4][time].imag() * s[y+1][time].imag(),
		     s[x+4][time].real() * s[y+1][time].imag() + s[x+4][time].imag() * s[y+1][time].real()
		     );
      
      c[x+4][y+2] += complex<float> (
		     s[x+4][time].real() * s[y+2][time].real() - s[x+4][time].imag() * s[y+2][time].imag(),
		     s[x+4][time].real() * s[y+2][time].imag() + s[x+4][time].imag() * s[y+2][time].real()
		     );
      
      c[x+4][y+3] += complex<float> (
		     s[x+4][time].real() * s[y+3][time].real() - s[x+4][time].imag() * s[y+3][time].imag(),
		     s[x+4][time].real() * s[y+3][time].imag() + s[x+4][time].imag() * s[y+3][time].real()
		     );
      
      c[x+4][y+4] += complex<float> (
		     s[x+4][time].real() * s[y+4][time].real() - s[x+4][time].imag() * s[y+4][time].imag(),
		     s[x+4][time].real() * s[y+4][time].imag() + s[x+4][time].imag() * s[y+4][time].real()
		     );
      
    }
    /* Process the leftovers */
    c[x  ][y  ] += complex<float> (
		   s[x  ][time].real() * s[y  ][time].real() - s[x  ][time].imag() * s[y  ][time].imag(),
		   s[x  ][time].real() * s[y  ][time].imag() + s[x  ][time].imag() * s[y  ][time].real()
		   );
    
    c[x+1][y  ] += complex<float> (
		   s[x+1][time].real() * s[y  ][time].real() - s[x+1][time].imag() * s[y  ][time].imag(),
		   s[x+1][time].real() * s[y  ][time].imag() + s[x+1][time].imag() * s[y  ][time].real()
		   );
    
    c[x+1][y+1] += complex<float> (
		   s[x+1][time].real() * s[y+1][time].real() - s[x+1][time].imag() * s[y+1][time].imag(),
		   s[x+1][time].real() * s[y+1][time].imag() + s[x+1][time].imag() * s[y+1][time].real()
		   );
    
    c[x+2][y  ] += complex<float> (
		   s[x+2][time].real() * s[y  ][time].real() - s[x+2][time].imag() * s[y  ][time].imag(),
		   s[x+2][time].real() * s[y  ][time].imag() + s[x+2][time].imag() * s[y  ][time].real()
		   );
    
    c[x+2][y+1] += complex<float> (
		   s[x+2][time].real() * s[y+1][time].real() - s[x+2][time].imag() * s[y+1][time].imag(),
		   s[x+2][time].real() * s[y+1][time].imag() + s[x+2][time].imag() * s[y+1][time].real()
		   );
    
    c[x+2][y+2] += complex<float> (
		   s[x+2][time].real() * s[y+2][time].real() - s[x+2][time].imag() * s[y+2][time].imag(),
		   s[x+2][time].real() * s[y+2][time].imag() + s[x+2][time].imag() * s[y+2][time].real()
		   );
    
    
    c[x+3][y  ] += complex<float> (
		   s[x+3][time].real() * s[y  ][time].real() - s[x+3][time].imag() * s[y  ][time].imag(),
		   s[x+3][time].real() * s[y  ][time].imag() + s[x+3][time].imag() * s[y  ][time].real()
		   );
    
    c[x+3][y+1] += complex<float> (
		   s[x+3][time].real() * s[y+1][time].real() - s[x+3][time].imag() * s[y+1][time].imag(),
		   s[x+3][time].real() * s[y+1][time].imag() + s[x+3][time].imag() * s[y+1][time].real()
		   );
    
    c[x+3][y+2] += complex<float> (
		   s[x+3][time].real() * s[y+2][time].real() - s[x+3][time].imag() * s[y+2][time].imag(),
		   s[x+3][time].real() * s[y+2][time].imag() + s[x+3][time].imag() * s[y+2][time].real()
		   );
    
    c[x+3][y+3] += complex<float> (
		   s[x+3][time].real() * s[y+3][time].real() - s[x+3][time].imag() * s[y+3][time].imag(),
		   s[x+3][time].real() * s[y+3][time].imag() + s[x+3][time].imag() * s[y+3][time].real()
		   );
    
    c[x+4][y  ] += complex<float> (
		   s[x+4][time].real() * s[y  ][time].real() - s[x+4][time].imag() * s[y  ][time].imag(),
		   s[x+4][time].real() * s[y  ][time].imag() + s[x+4][time].imag() * s[y  ][time].real()
		   );
    
    c[x+4][y+1] += complex<float> (
		   s[x+4][time].real() * s[y+1][time].real() - s[x+4][time].imag() * s[y+1][time].imag(),
		   s[x+4][time].real() * s[y+1][time].imag() + s[x+4][time].imag() * s[y+1][time].real()
		   );
    
    c[x+4][y+2] += complex<float> (
		   s[x+4][time].real() * s[y+2][time].real() - s[x+4][time].imag() * s[y+2][time].imag(),
		   s[x+4][time].real() * s[y+2][time].imag() + s[x+4][time].imag() * s[y+2][time].real()
		   );
    
    c[x+4][y+3] += complex<float> (
		   s[x+4][time].real() * s[y+3][time].real() - s[x+4][time].imag() * s[y+3][time].imag(),
		   s[x+4][time].real() * s[y+3][time].imag() + s[x+4][time].imag() * s[y+3][time].real()
		   );
    
    c[x+4][y+4] += complex<float> (
		   s[x+4][time].real() * s[y+4][time].real() - s[x+4][time].imag() * s[y+4][time].imag(),
		   s[x+4][time].real() * s[y+4][time].imag() + s[x+4][time].imag() * s[y+4][time].real()
		   );
    
    
    }
  }
}

void WH_Correlate::master() {

  int task_id      = 0;
  int result_id    = 0;
  int active_nodes = 0;
  int sending_node = 0;
#ifdef __BLRTS__

  MPI_Status status;
  
  /* Send all nodes with (rank > 0) a TASK id and the corresponding data */ 
  for (int i = 1; i < mysize; i++) {           
    MPI_Send(&task_id, 
	     1, 
	     MPI_INT, 
	     i, 
	     SEND_TASK_ID, 
	     MPI_COMM_WORLD);

    MPI_Send(&sig_buf, //+(task_id*NSTATIONS*NSAMPLES),
	     NSTATIONS*NSAMPLES,
	     my_complex,
	     i, 
	     SEND_TASK_DATA,
	     MPI_COMM_WORLD);
    
    task_id++;
    active_nodes++;
  }

  while (active_nodes > 0) {

    if (task_id < NCHANNELS) {
      /* There are still frequency channels to process */
      MPI_Recv(&result_id, 
	       1, 
	       MPI_INT,
	       MPI_ANY_SOURCE,
	       SEND_RESULT_ID,
	       MPI_COMM_WORLD,
	       &status);
      
      sending_node = status.MPI_SOURCE;

      MPI_Recv(&corr_buf,//+(result_id*NSTATIONS*NSTATIONS), 
	       NSTATIONS*NSTATIONS,
	       my_complex,
	       sending_node,
	       SEND_RESULT_DATA,
	       MPI_COMM_WORLD,
	       &status);

      /* Give the slave a new task id and corresponding data right away */
      MPI_Send(&task_id,
	       1, 
	       MPI_INT,
	       sending_node,
	       SEND_TASK_ID,
	       MPI_COMM_WORLD);

      MPI_Send(&sig_buf,//+(task_id*NSTATIONS*NSAMPLES),
	       NSTATIONS*NSAMPLES, 
	       my_complex,
	       sending_node,
	       SEND_TASK_DATA,
	       MPI_COMM_WORLD);

      task_id++;

    } else {
      
      /* all data has been sent to the slaves. Any slave that transmits data  */
      /* is stopped once the data has been received.                          */ 
      
      /* Receive a result id from any source */
      MPI_Recv(&result_id, 
	       1, 
	       MPI_INT,
	       MPI_ANY_SOURCE,
	       SEND_RESULT_ID,
	       MPI_COMM_WORLD,
	       &status);

      sending_node = status.MPI_SOURCE;

      /* Receive the corresponding result matrix */ 
      MPI_Recv(&corr_buf,//+(result_id*NSTATIONS*NSTATIONS),
	       NSTATIONS*NSTATIONS,
	       my_complex,
	       sending_node,
	       SEND_RESULT_DATA,
	       MPI_COMM_WORLD,
	       &status);

      /* Once the data has been received, stop the slave */
      MPI_Send(&task_id,
	       1, 
	       MPI_INT,
	       sending_node,
	       STOP_TAG,
	       MPI_COMM_WORLD);
      active_nodes--;
    }
  }
#endif
}

void WH_Correlate::slave(const int rank) {

  complex<float> signal_buffer[NSTATIONS][NSAMPLES];
  complex<float> corr_buffer  [NSTATIONS][NSTATIONS];
  
  int id;

#ifdef __BLRTS__

  MPI_Status status;
  
  while (1) {

    /* receive a task id number */
    MPI_Recv(&id,
	     1,
	     MPI_INT,
	     0,
	     MPI_ANY_TAG,               // can either be SEND_TASK_ID or STOP_TAG
	     MPI_COMM_WORLD,
	     &status);

    if (status.MPI_TAG == SEND_TASK_ID) {
     
      /* Receive task data */
      MPI_Recv(&signal_buffer,
	       NSTATIONS*NSAMPLES,
	       my_complex,
	       0, 
	       SEND_TASK_DATA,
	       MPI_COMM_WORLD,
	       &status);

      /* Compute complete integrated correlation matrix */      
//       WH_Correlate::correlator_core(signal_buffer, corr_buffer);
      WH_Correlate::correlator_core_unrolled(signal_buffer, corr_buffer);
      
      /* Send result ID to master */

      
      MPI_Send(&id,
	       1,
	       MPI_INT,
	       0,
	       SEND_RESULT_ID,
	       MPI_COMM_WORLD);

      /* Send result data to master */
      MPI_Send(&corr_buffer,
	       NSTATIONS*NSTATIONS,
	       my_complex,
	       0,
	       SEND_RESULT_DATA,
	       MPI_COMM_WORLD);
    } else {
      /* status.MPI_TAG == STOP_TAG */
      /* all data has been processed, stop the slave */ 
      break;
    }
  }
#endif
}
