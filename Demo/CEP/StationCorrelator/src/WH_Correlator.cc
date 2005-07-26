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
//#include <hummer_builtin.h>
#endif 


// Timing doesn't work because it uses MPI_Reduce.
// A new communicator should be made that contains only the correlator nodes
#define DO_TIMING_NOT_DEFINED
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

  // reset integrator.
  memset(outDH->getBuffer(), 
	 0,
	 outDH->getBufSize()*sizeof(DH_Vis::BufferType));



  //
  // this block of code does the cast from complex<uint16> to complex<float>
  // 
  // using builtin complex type
  // this will work for both GNU and IBM compilers. The Intel compiler only recognizes the older __complex__ type
  DH_CorrCube::BufferPrimitive* in_ptr = (DH_CorrCube::BufferPrimitive*) inDH->getBuffer();
  _Complex float* in_buffer = new _Complex float[inBufSize];
  for ( int i = 0; i < inBufSize; i++ ) {
    __real__ *(in_buffer+i) =  *(in_ptr+2*i);    
    __imag__ *(in_buffer+i) =  *(in_ptr+2*i+1);
  }

  //
  // This is the actual correlator
  // Note that there is both a machine independent correlator as well as a BlueGene
  // specific implementation.
  // 


//   complex<double> * out_ptr;
  _Complex double * out_ptr;

  for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
    int c_addr = itsNpolarisations*itsNelements*itsNsamples*fchannel;
    for (int station1 = 0; station1 < itsNelements; station1++) {
      for (int station2 = 0; station2 <= station1; station2++) {
	int s1_addr = c_addr+itsNpolarisations*station1;
	int s2_addr = c_addr+itsNpolarisations*station2;

 	out_ptr = reinterpret_cast<_Complex double*> (outDH->getBufferElement(station1, station2, fchannel, 0));
//	out_ptr = outDH->getBufferElement(station1, station2, fchannel, 0);

	for (int sample = 0; sample < itsNsamples; sample++) {
	  *out_ptr     += *(in_buffer+s1_addr) * conj( *(in_buffer+s2_addr));     // XX
	  *(out_ptr+1) += *(in_buffer+s1_addr) * conj(*(in_buffer+s2_addr+1));   // XY
	  *(out_ptr+2) += *(in_buffer+s1_addr+1) * conj(*(in_buffer+s2_addr));   // YX
	  *(out_ptr+3) += *(in_buffer+s1_addr+1) * conj(*(in_buffer+s2_addr+1)); // YY
	  s1_addr+=itsNpolarisations*itsNelements; 
	  s2_addr+=itsNpolarisations*itsNelements;
	} // sample
      } // station2
    } // station1
  } // fchannel


  delete[](in_buffer);
  
}

void WH_Correlator::dump() const {

}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
