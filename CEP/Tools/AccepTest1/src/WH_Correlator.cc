//#  WH_Correlator.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <stdio.h>

// General includes
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

using namespace LOFAR;

WH_Correlator::WH_Correlator(const string& name, 
			     const int    elements,
			     const int    samples,
			     const int    channels, 
			     const int    polarisations,
			     const int    targets) 
  : WorkHolder( 1, 1, name, "WH_Correlator"),
    itsNelements(elements),
    itsNsamples (samples),
    itsNchannels(channels),
    itsNpolarisations(polarisations),
    itsNtargets (targets)
{

  getDataManager().addInDataHolder(0, new DH_CorrCube("in", 
						      elements, 
						      samples, 
						      channels,
						      polarisations));

  getDataManager().addOutDataHolder(0, new DH_Vis("out", 
						  elements, channels, polarisations));

  t_start.tv_sec = 0;
  t_start.tv_usec = 0;

  bandwidth=0.0;
  agg_bandwidth=0.0;
}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct (const string& name, 
				      const int    nelements, 
				      const int    nsamples,
				      const int    nchannels,
				      const int    npolarisations,
				      const int    ntargets)
{
  return new WH_Correlator(name, nelements, nsamples, nchannels, npolarisations, ntargets);
}

WH_Correlator* WH_Correlator::make (const string& name) {
  return new WH_Correlator(name,
			   itsNelements,
			   itsNsamples,
			   itsNchannels, 
			   itsNpolarisations,
			   itsNtargets);
}

void WH_Correlator::process() {
  double starttime, stoptime, cmults;
  // variables to store prefetched antenna data
#ifdef HAVE_BGL
  _Complex float *s1_val_0, *s1_val_1;
  _Complex float *s2_val_0, *s2_val_1;
#else
  complex<float> *s1_val_0, *s1_val_1;
  complex<float> *s2_val_0, *s2_val_1;
#endif


#ifdef DO_TIMING
  if (t_start.tv_sec != 0 && t_start.tv_usec != 0) {
    gettimeofday(&t_stop, NULL);
    
    bandwidth = 8.0 *
      ((itsNchannels*itsNelements*itsNsamples*itsNpolarisations*sizeof(DH_CorrCube::BufferType)) +
       (itsNchannels*itsNelements*itsNelements*itsNpolarisations*sizeof(DH_Vis::BufferType))) /
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


  DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  // reset integrator.
  memset(outDH->getBuffer(), 
	 0,
	 itsNchannels*itsNelements*itsNelements*itsNpolarisations*itsNpolarisations*sizeof(DH_Vis::BufferType));



  //
  // this block of code does the cast from complex<uint16> to complex<float>
  // 
  // This is an uint16 pointer

  // consider the input buffer of complex<uint16> to be uint16 of twice that size
  // we can now offer the compiler a single for loop which has great potential to unroll
#ifdef HAVE_BGL
  DH_CorrCube::BufferPrimitive* in_ptr = (DH_CorrCube::BufferPrimitive*) inDH->getBuffer();
  _Complex float* in_buffer = new _Complex float[2*inDH->getBufSize()];
  for ( unsigned int i = 0; i < inDH->getBufSize(); i++ ) {
    *(in_buffer+i) = static_cast<_Complex float> ( *(in_ptr+i) ); 
  }
#else
  DH_CorrCube::BufferPrimitive* in_ptr = (DH_CorrCube::BufferPrimitive*) inDH->getBuffer();
  // The float pointer is explicit, since DH_Vis is now complex<double>
  complex<float>*  in_buffer = new complex<float>[2*inDH->getBufSize()];
  for ( unsigned int i = 0; i < inDH->getBufSize(); i++ ) {
    *(in_buffer+i) = static_cast<complex<float> > ( *(in_ptr+i) ); 
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
      int s1_addr = c_addr+itsNsamples*itsNpolarisations*station1;
      for (int station2 = 0; station2 <= station1; station2++) {
	int s2_addr = c_addr+itsNsamples*itsNpolarisations*station2;
	out_ptr = reinterpret_cast<_Complex double*> (outDH->getBufferElement(station1, station2, fchannel, 0));
	for (int sample = 0; sample < itsNsamples; sample++) {
#if 0
	  *out_ptr   += *(in_buffer+s1_addr) * *(in_buffer+s2_addr);     // XX
 	  out_ptr++;
	  *out_ptr   += *(in_buffer+s1_addr) * *(in_buffer+s2_addr+1);   // XY
 	  out_ptr++;
	  *out_ptr   += *(in_buffer+s1_addr+1) * *(in_buffer+s2_addr);   // YX
 	  *out_ptr++;
	  *out_ptr   += *(in_buffer+s1_addr+1) * *(in_buffer+s2_addr+1); // YY
#endif 

#if 1
	  // XX
	  *out_ptr = __fxcpmadd( *out_ptr, *(in_buffer+s1_addr), __real__ *(in_buffer+s2_addr) );
	  *out_ptr = __fxcxnpma( *out_ptr, *(in_buffer+s1_addr), __imag__ *(in_buffer+s2_addr) );
	  out_ptr++;
	  // XY
	  *out_ptr = __fxcpmadd( *out_ptr, *(in_buffer+s1_addr), __real__ *(in_buffer+s2_addr+1) );
	  *out_ptr = __fxcxnpma( *out_ptr, *(in_buffer+s1_addr), __imag__ *(in_buffer+s2_addr+1) );
	  out_ptr++;
	  // YX
	  *out_ptr = __fxcpmadd( *out_ptr, *(in_buffer+s1_addr+1), __real__ *(in_buffer+s2_addr) );
	  *out_ptr = __fxcxnpma( *out_ptr, *(in_buffer+s1_addr+1), __imag__ *(in_buffer+s2_addr) );
	  out_ptr++;
	  // YY
	  *out_ptr = __fxcpmadd( *out_ptr, *(in_buffer+s1_addr+1), __real__ *(in_buffer+s2_addr+1) );
	  *out_ptr = __fxcxnpma( *out_ptr, *(in_buffer+s1_addr+1), __imag__ *(in_buffer+s2_addr+1) );
#endif 

	} // sample
      } // station2
    } // station1
  } // fchannel


#else

  for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
    int c_addr = itsNpolarisations*itsNelements*itsNsamples*fchannel;
    for (int station1 = 0; station1 < itsNelements; station1++) {
      int s1_addr = c_addr+itsNsamples*itsNpolarisations*station1;
      for (int station2 = 0; station2 <= station1; station2++) {
	int s2_addr = c_addr+itsNsamples*itsNpolarisations*station2;
	DH_Vis::BufferType* out_ptr = outDH->getBufferElement(station1, station2, fchannel, 0);
	for (int sample = 0; sample < itsNsamples; sample++) {

	  *out_ptr   += *(in_buffer+s1_addr) * *(in_buffer+s2_addr);     // XX
	  out_ptr++;
	  *out_ptr   += *(in_buffer+s1_addr) * *(in_buffer+s2_addr+1);   // XY
	  out_ptr++;
	  *out_ptr   += *(in_buffer+s1_addr+1) * *(in_buffer+s2_addr);   // YX
	  *out_ptr++;
	  *out_ptr   += *(in_buffer+s1_addr+1) * *(in_buffer+s2_addr+1); // YY

	} // sample
      } // station2
    } // station1
  } // fchannel

#endif // HAVE_BGL
#ifdef DO_TIMING
  stoptime = timer();
#endif

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
