//#  filename.cc: generic correlator class
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

#include <lofar_config.h>
#include <stdio.h>

// General includes
#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

// Application specific includes
#include <DH_Vis.h>
#include <DH_CorrCube.h>
#include <WH_Correlator.h>

#ifdef HAVE_BGL
// cheat by including the entire hummer_builtin.h file
#include <hummer_builtin.h>
#endif 


#define DO_TIMING
#define USE_BUILTIN

using namespace LOFAR;

WH_Correlator::WH_Correlator(const string& name, 
			     const KeyValueMap& kvm)
  : WorkHolder( 1, 1, name, "WH_Correlator"),
    itsKVM    (kvm)
{
  itsNelements = itsKVM.getInt("NoWH_RSP", 92);
  itsNsamples  = itsKVM.getInt("samples", 256000);
  itsNchannels = itsKVM.getInt("NoRSPBeamlets", 92)/itsKVM.getInt("NoWH_Correlator", 92);
  itsNpolarisations = itsKVM.getInt("polarisations", 2);
  itsNtargets = 0; // not used?

  getDataManager().addInDataHolder(0, new DH_CorrCube("in", 
						      itsNelements, 
						      itsNsamples, 
						      itsNchannels,
						      itsNpolarisations));

  getDataManager().addOutDataHolder(0, new DH_Vis("out", 
						  itsNelements, 
						  itsNchannels, 
						  itsNpolarisations));

  t_start.tv_sec = 0;
  t_start.tv_usec = 0;

  bandwidth=0.0;
  agg_bandwidth=0.0;

  corr_perf=0.0;
}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct (const string& name, 
				      const KeyValueMap& kvm)
{
  return new WH_Correlator(name, kvm);
}

WH_Correlator* WH_Correlator::make (const string& name) {
  return new WH_Correlator(name, itsKVM);
}

void WH_Correlator::process() {
  double starttime, stoptime, cmults;

  DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  const int inBufSize = inDH->getBufSize();

#ifdef DO_TIMING
  if (t_start.tv_sec != 0 && t_start.tv_usec != 0) {
    gettimeofday(&t_stop, NULL);
    
    bandwidth = 8.0 *
      ((inBufSize*sizeof(DH_CorrCube::BufferType)) +
       (outDH->getBufSize()*sizeof(DH_Vis::BufferType))) /
      (t_stop.tv_sec + 1.0e-6*t_stop.tv_usec - 
       t_start.tv_sec + 1.0e-6*t_start.tv_usec);

#ifdef HAVE_MPI
    // collect partial bandwidths from all nodes
    MPI_Reduce(&bandwidth, &agg_bandwidth, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (TH_MPI::getCurrentRank() == 0) {
    }
#endif
  }
#endif 

  // reset integrator.
  memset(outDH->getBuffer(), 
	 0,
	 outDH->getBufSize()*sizeof(DH_Vis::BufferType));



  //
  // this block of code does the cast from complex<uint16> to complex<float>
  // 
#if 1
  // using builtin complex type
  // this will work for both GNU and IBM compilers. The Intel compiler only recognizes the older __complex__ type
  DH_CorrCube::BufferPrimitive* in_ptr = (DH_CorrCube::BufferPrimitive*) inDH->getBuffer();
  _Complex float* in_buffer = new _Complex float[inBufSize];
  for ( int i = 0; i < inBufSize; i++ ) {
    __real__ *(in_buffer+i) =  *(in_ptr+2*i);    
    __imag__ *(in_buffer+i) =  *(in_ptr+2*i+1);
  }
#else
  // using stl templated complex type
  DH_CorrCube::BufferType* in_ptr = (DH_CorrCube::BufferType*) inDH->getBuffer();
  // The float pointer is explicit, since DH_Vis is now complex<double>
  complex<float>*  in_buffer = new complex<float>[inBufSize];
  for ( int i = 0; i < inBufSize; i++ ) {
    *(in_buffer+i) = *(in_ptr+i); 
  }
#endif 

  //
  // This is the actual correlator
  // Note that there is both a machine independent correlator as well as a BlueGene
  // specific implementation.
  // 

#ifdef DO_TIMING
  starttime = timer();
#endif
#ifdef HAVE_BGL
  
  // BlueGene/L specific correlator code
  // compared to earlier versions I have:
  // * removed the prefetch statements since the compiler should be able to do this anyway
  // * cleaned up the code by moving the platform independent stuff to it's own nested loop
  // * changed the loop order to minimize stride size

  _Complex double * out_ptr;

  __alignx(16, out_ptr);
  __alignx(8 , in_buffer);

  for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
    int c_addr = itsNpolarisations*itsNelements*itsNsamples*fchannel;
    for (int station1 = 0; station1 < itsNelements; station1++) {
      for (int station2 = 0; station2 <= station1; station2++) {
	int s1_addr = c_addr+itsNsamples*itsNpolarisations*station1;
	int s2_addr = c_addr+itsNsamples*itsNpolarisations*station2;
	out_ptr = reinterpret_cast<_Complex double*> (outDH->getBufferElement(station1, station2, fchannel, 0));
#pragma unroll(10)
	for (int sample = 0; sample < itsNsamples; sample++) {
#if 1
	  *out_ptr     += *(in_buffer+s1_addr+sample) * *(in_buffer+s2_addr+sample);     // XX	  
	  *(out_ptr+1) += *(in_buffer+s1_addr+sample) * *(in_buffer+s2_addr+sample+1);   // XY
	  *(out_ptr+2) += *(in_buffer+s1_addr+sample+1) * *(in_buffer+s2_addr+sample);   // YX
	  *(out_ptr+3) += *(in_buffer+s1_addr+sample+1) * *(in_buffer+s2_addr+sample+1); // YY
#else
	  // XX
	  *out_ptr = __fxcpmadd( *out_ptr, *(in_buffer+s1_addr), __real__ *(in_buffer+s2_addr) );
	  *out_ptr = __fxcxnpma( *out_ptr, *(in_buffer+s1_addr), __imag__ *(in_buffer+s2_addr) );
	  // XY
	  *(out_ptr+1) = __fxcpmadd( *(out_ptr+1), *(in_buffer+s1_addr), __real__ *(in_buffer+s2_addr+1) );
	  *(out_ptr+1) = __fxcxnpma( *(out_ptr+1), *(in_buffer+s1_addr), __imag__ *(in_buffer+s2_addr+1) );
	  // YX
	  *(out_ptr+2) = __fxcpmadd( *(out_ptr+2), *(in_buffer+s1_addr+1), __real__ *(in_buffer+s2_addr) );
	  *(out_ptr+2) = __fxcxnpma( *(out_ptr+2), *(in_buffer+s1_addr+1), __imag__ *(in_buffer+s2_addr) );
	  // YY
	  *(out_ptr+3) = __fxcpmadd( *(out_ptr+3), *(in_buffer+s1_addr+1), __real__ *(in_buffer+s2_addr+1) );
	  *(out_ptr+3) = __fxcxnpma( *(out_ptr+3), *(in_buffer+s1_addr+1), __imag__ *(in_buffer+s2_addr+1) );
#endif 
	  s1_addr++;
	  s2_addr++;

	} // sample
      } // station2
    } // station1
  } // fchannel

#else

//   complex<double> * out_ptr;
  _Complex double * out_ptr;

  for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
    int c_addr = itsNpolarisations*itsNelements*itsNsamples*fchannel;
    for (int station1 = 0; station1 < itsNelements; station1++) {
      for (int station2 = 0; station2 <= station1; station2++) {
	int s1_addr = c_addr+itsNsamples*itsNpolarisations*station1;
	int s2_addr = c_addr+itsNsamples*itsNpolarisations*station2;

 	out_ptr = reinterpret_cast<_Complex double*> (outDH->getBufferElement(station1, station2, fchannel, 0));
//	out_ptr = outDH->getBufferElement(station1, station2, fchannel, 0);

	for (int sample = 0; sample < itsNsamples; sample++) {
	  *out_ptr     += *(in_buffer+s1_addr+sample) * *(in_buffer+s2_addr+sample);     // XX
	  *(out_ptr+1) += *(in_buffer+s1_addr+sample) * *(in_buffer+s2_addr+sample+1);   // XY
	  *(out_ptr+2) += *(in_buffer+s1_addr+sample+1) * *(in_buffer+s2_addr+sample);   // YX
	  *(out_ptr+3) += *(in_buffer+s1_addr+sample+1) * *(in_buffer+s2_addr+sample+1); // YY
	  s1_addr++; 
	  s2_addr++;
	} // sample
      } // station2
    } // station1
  } // fchannel

#endif // HAVE_BGL
#ifdef DO_TIMING
  stoptime = timer();
#endif
  
  delete[](in_buffer);

#ifdef DO_TIMING
#ifdef HAVE_MPI
  double elapsed_time = (stoptime-starttime);
  double min_time;

  // we're selecting the highest performance figure of the nodes. Since BG/L is a hard real-time 
  // system, we expect these to be the same for each node. While debugging we're not interrested in 
  // transient effects on the nodes, so the maximum performance is a reasonable estimate of the real 
  // performance of the application.

  cmults = itsNpolarisations * itsNpolarisations * itsNsamples * itsNchannels * itsNelements * itsNelements / 2;
  MPI_Reduce(&elapsed_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

  if ((TH_MPI::getCurrentRank() == 0) && (t_start.tv_sec != 0) && (t_start.tv_usec != 0)) {
    corr_perf = 1.0e-6*cmults/min_time;
  }

#endif // HAVE_MPI
  gettimeofday(&t_start, NULL);
#endif // DO_TIMING
}

void WH_Correlator::dump() {

}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
