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
#define DO_TIMING

#ifdef DO_TIMING
  double agg_bandwidth = 0.0;
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
      cout << itsNsamples  << " " ;
      cout << itsNchannels << " " ;
      cout << itsNelements << " " ;
      cout << itsNpolarisations << " " ;
      cout << ((itsNchannels*itsNelements*itsNsamples*itsNpolarisations*sizeof(DH_CorrCube::BufferType)) + 
	(itsNchannels*itsNelements*itsNelements*itsNpolarisations*sizeof(DH_Vis::BufferType)))/ (1024.0*1024.0) << " ";
      cout << agg_bandwidth/(1024.0*1024.0) << " Mbps " ;
      cout << (100.0*agg_bandwidth)/(1024.0*1024.0*1024.0)<< "%" << " ";
    }
#endif
  }
#endif 

  DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  // reset integrator.
  memset(outDH->getBuffer(), 
	 0,
	 itsNchannels*itsNelements*itsNelements*itsNpolarisations*sizeof(DH_Vis::BufferType));


#ifdef DO_TIMING
  starttime = timer();
#endif


  //
  // this block of code does the cast from complex<uint16> to complex<float>
  // 
  uint16* in_ptr = (uint16*) inDH->getBuffer();
  float*  in_buffer = new float[2*inDH->getBufSize()];

  // consider the input buffer of complex<uint16> to be uint16 of twice that size
  // we can now offer the compiler a single for loop which has great potential to unroll
  for ( unsigned int i = 0; i < 2*inDH->getBufSize(); i++ ) {
    *(in_buffer+i) = static_cast<float> ( *(in_ptr+i) ); 
  }

   DH_Vis::BufferType s1_val, s2_val;
   //   DH_Vis::BufferType* out_ptr = outDH->getBuffer();

  for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
#ifdef HAVE_MPE
    MPE_Log_event(1, sample, "correlating"); 
#endif
    for (int sample = 0; sample < itsNsamples; sample++) {
      int sample_addr = itsNpolarisations*itsNelements*itsNsamples*fchannel+
	itsNpolarisations*itsNelements*sample;

      for (int   station1 = 0; station1 < itsNelements; station1++) {
	int s1_addr = sample_addr+itsNpolarisations*station1;

	for (int station2 = 0; station2 <= station1; station2++) {
	  int s2_addr = sample_addr+itsNpolarisations*station2;

	  for (int polarisation = 0; polarisation < itsNpolarisations; polarisation++) {

 	    // prefetch from L1
 	    s1_val = *( in_buffer + s1_addr + polarisation );
 	    s2_val = *( in_buffer + s2_addr + polarisation );

#ifdef HAVE_BGL

	    // load prefetched values into FPU
	    __lfps((float*) s1_val);
	    __lfps((float*) s2_val);
 
	    // do the actual complex fused multiply-add
	    // understand that. Even though it's an inlined function, it may still be too expensive
	    __fxcxnpma(out_ptr++, (float*)s1_val, (float*)s2_val);
	    // note that the output buffer is assumed to be contiguous

#else 
	    // this is purely functional code, very expensive and slow as hell
	    outDH->addBufferElementVal(station1, station2, fchannel, polarisation,
				       s1_val * s2_val);
#endif 	    
 	  }
	}
      }
    }
#ifdef HAVE_MPE
    MPE_Log_event(2, sample, "correlated");
#endif 
  }

#ifdef DO_TIMING
  stoptime = timer();
#endif

#ifdef DO_TIMING
#ifdef HAVE_MPI
  double time = (stoptime-starttime);
  double min_time;

  // we're selecting the highest performance figure of the nodes. Since BG/L is a hard real-time 
  // system, we expect these to be the same for each node. While debugging we're not interrested in 
  // transient effects on the nodes, so the maximum performance is a reasonable estimate of the real 
  // performance of the application.

  cmults = itsNsamples * itsNchannels * (itsNelements*itsNelements/2 + ceil(itsNelements/2.0));
  MPI_Reduce(&time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  
  if ((TH_MPI::getCurrentRank() == 0) && (t_start.tv_sec != 0) && (t_start.tv_usec != 0)) {
    cout << 1.0e-6*cmults/min_time << " Mcprod/sec" << endl;
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
