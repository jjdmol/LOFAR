//  WH_Correlate.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <Common/Debug.h>

// CEPFrame general includes
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include <WH_Correlate.h>
#include <BlueGeneDefinitions.h>

namespace LOFAR
{

WH_Correlate::WH_Correlate (const string& name,
			    unsigned int channels)
  : WorkHolder    (channels, channels, name,"WH_Correlate")
{
  char str[8];
  // create the input dataholders
  for (unsigned int i=0; i<channels; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_CorrCube (string("out_") + str));
  }
  // create the output dataholders
  for (unsigned int i=0; i<channels; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_Vis (string("out_") + str));
  }
}

WH_Correlate::~WH_Correlate()
{
}

WorkHolder* WH_Correlate::construct (const string& name, 
				     unsigned int channels)
{
  return new WH_Correlate (name, channels);
}

WH_Correlate* WH_Correlate::make (const string& name)
{
  return new WH_Correlate (name, 
			   getDataManager().getInputs());
}

void WH_Correlate::process()
{
  
  TRACER4("WH_Correlate::Process()");

#ifdef __BLRTS__

  int myrank;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	 
  if (myrank == 0) {
    // I'm the master. Read signal cube from input, start the master process and
    // write the resulting matrix to the output.

    DH_CorrCube* InDHptr;
    DH_Vis*      OutDHptr;

    InDHptr  = (DH_CorrCube*)getDataManager().getInHolder(0);
    OutDHptr = (DH_Vis*)getDataManager().getOutHolder(0);

    // read the signal cube from the input
    memcpy(signal, 
	   InDHptr->getBuffer(), 
	   itsNelements*itsNsamples*itsNchannels*sizeof(DH_CorrCube::BufferType));
    getDataManager().readyWithInHolder(0);

    // run the master process
    WH_Correlate::master();

    // copy the correlation matrices to the output   
    memcpy(OutDHptr->getBuffer(), 
	   corr, 
	   itsNelements*itsNelements*itsNchannels*sizeof(DH_Vis::BufferType));
    getDataManager().readyWithOutHolder(0);

  } else {
    // as the slave I only need to start the slave process
    WH_Correlate::slave(myrank);
  }

#endif
}	 


void WH_Correlate::dump()
{
}


void WH_Correlate::correlator_core(complex<float> signal[itsNelements][itsNsamples], 
				   complex<float> corr[itsNelements][itsNelements]) {

  int x, y;
  for (int time = 0; time < itsNsamples; time++) {
    for (x = 0; x < itsNelements; x++) {
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

void WH_Correlate::correlator_core_unrolled(complex<float> s[itsNelements][itsNsamples],
					    complex<float> c[itsNelements][itsNelements]) {

  int loop = 5;
  int x, y;

  for ( int time = 0; time < itsNsamples; time++) {
    for ( x = 0; (x+loop) < itsNelements; x += loop ) {
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

  int mysize;

  int result_id;

  complex<float> sig_buffer[itsNelements][itsNsamples];
  complex<float> res_buffer[itsNelements][itsNelements];
  
  MPI_Status status;
  int id_buffer = 0;
  int active_nodes = 0;

  MPI_Comm_size(MPI_COMM_WORLD, &mysize);

  /* Send all nodes a TEST ID */
  for (int i = 1; i < mysize; i++) {
    MPI_Send(&id_buffer, 1 , MPI_INT, i, SEND_TASK_ID, MPI_COMM_WORLD);
    MPI_Send(&sig_buffer, itsNelements*itsNsamples, 
	     my_complex, i, SEND_TASK_DATA, MPI_COMM_WORLD);
    id_buffer = id_buffer + 1;
    active_nodes++;
  }

  while (active_nodes > 0) {
    
    if (id_buffer < itsNchannels) {

      /* Receive a result ID from any source */ 
      MPI_Recv(&result_id, 
	       1, 
	       MPI_INT, 
	       MPI_ANY_SOURCE, 
	       SEND_RESULT_ID, 
	       MPI_COMM_WORLD, 
	       &status);

      int cur_source = status.MPI_SOURCE;

      /* Receive a result matrix */
      MPI_Recv(&res_buffer, 
	       itsNelements*itsNelements, 
	       my_complex, 
	       cur_source, 
	       SEND_RESULT_DATA, 
	       MPI_COMM_WORLD, 
	       &status) ;

      /* The slave gets a new task right away */
      MPI_Send(&id_buffer, 
	       1, 
	       MPI_INT, 
	       cur_source, 
	       SEND_TASK_ID,
	       MPI_COMM_WORLD);

      MPI_Send(&sig_buffer,
	       itsNelements*itsNsamples,
	       my_complex, 
	       cur_source, 
	       SEND_TASK_DATA, 
	       MPI_COMM_WORLD);

      id_buffer++;
    } else {

      /* Receive a result id from any source */
      MPI_Recv(&result_id, 
	       1, 
	       MPI_INT, 
	       MPI_ANY_SOURCE, 
	       SEND_RESULT_ID, 
	       MPI_COMM_WORLD, 
	       &status);

      int cur_source = status.MPI_SOURCE;

      /* Receive a result matrix */
      MPI_Recv(&res_buffer, 
	       itsNelements*itsNelements, 
	       my_complex, 
	       cur_source, 
	       SEND_RESULT_DATA, 
	       MPI_COMM_WORLD, 
	       &status) ;
      
      MPI_Send(&id_buffer, 
	       1, 
	       MPI_INT,
	       cur_source, 
	       STOP_TAG, 
	       MPI_COMM_WORLD);
      active_nodes--;
    }
  }
}

void WH_Correlate::slave(const int rank) {

  complex<float> signal[itsNelements][itsNsamples];
  complex<float> corr  [itsNelements][itsNelements];

  MPI_Status status;

  int id;

  while (1) {
    
    MPI_Recv(&id,
	     1,
	     MPI_INT, 
	     0, 
	     MPI_ANY_TAG,    // can be either SEND_TASK_ID or STOP_TAG
	     MPI_COMM_WORLD,
	     &status);
    
    if (status.MPI_TAG == SEND_TASK_ID) {

      /* Receive task data */
      MPI_Recv(&signal,
	       itsNelements*itsNsamples,
	       my_complex,
	       0,
	       SEND_TASK_DATA,
	       MPI_COMM_WORLD,
	       &status);

      /* compute */

      /* Send result ID to master */
      MPI_Send(&id,
	       1,
	       MPI_INT,
	       0,
	       SEND_RESULT_ID,
	       MPI_COMM_WORLD);
      
      /* Send result data */
      MPI_Send(&corr, 
	       itsNelements*itsNelements, 
	       my_complex, 
	       0,
	       SEND_RESULT_DATA,
	       MPI_COMM_WORLD);
    } else {
      /* status.MPI_TAG == STOP_TAG */
      break;
    }
  }
}


}// namespace LOFAR
