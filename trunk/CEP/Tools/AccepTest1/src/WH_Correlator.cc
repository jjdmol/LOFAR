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
  _Complex float *s1_val_0, *s1_val_1;
  _Complex float *s2_val_0, *s2_val_1;

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
  // This is a uint16 pointer
  DH_CorrCube::BufferPrimitive* in_ptr = (DH_CorrCube::BufferPrimitive*) inDH->getBuffer();
  // The float pointer is explicit, since DH_Vis is complex<double>
  float*  in_buffer = new float[2*inDH->getBufSize()];

  // consider the input buffer of complex<uint16> to be uint16 of twice that size
  // we can now offer the compiler a single for loop which has great potential to unroll
  for ( unsigned int i = 0; i < 2*inDH->getBufSize(); i++ ) {
    *(in_buffer+i) = static_cast<float> ( *(in_ptr+i) ); 
  }

  //
  // This is the actual correlator
  // Note that there is both a general correlator as well as a BlueGene specific
  // implementation.
  // 
#ifdef HAVE_BGL
  // complex<double> pointer to the output buffer
  _Complex double * out_ptr = reinterpret_cast<_Complex double*> ( outDH->getBuffer() );
#endif

  for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
#ifdef HAVE_MPE
    MPE_Log_event(1, sample, "correlating"); 
#endif
    for (int sample = 0; sample < itsNsamples; sample++) {
      int sample_addr = itsNpolarisations*itsNelements*itsNsamples*fchannel+
	itsNpolarisations*itsNelements*sample;

      for (int   station1 = 0; station1 < itsNelements; station1++) {
	int s1_addr = sample_addr+itsNpolarisations*station1;
	// prefetch station1, both polarisation 0 and 1
	s1_val_0 = reinterpret_cast<_Complex float*>( in_buffer + s1_addr );
	s1_val_1 = reinterpret_cast<_Complex float*>( in_buffer + s1_addr + 1 );

	for (int station2 = 0; station2 <= station1; station2++) {
	  int s2_addr = sample_addr+itsNpolarisations*station2;

 	    // prefetch station2, both polarisation 0 and 1
  	    s2_val_0 = reinterpret_cast<_Complex float*>( in_buffer + s2_addr );
	    s2_val_1 = reinterpret_cast<_Complex float*>( in_buffer + s2_addr + 1 ) ;

#ifdef HAVE_BGL
	    // load prefetched values into FPU
	    // (now done inside the intrinsic)
//  	    __lfps((float*) s1_val_0);
//   	    __lfps(&__real__ *s2_val_0);

	    // do the actual complex fused multiply-add (polarisation 0)
	    // note that s1_val and s2_val are pointer of the type complex<float>
	    // this may be incompatible with the __real__ and __imag__ macro
	    // - we may want to use the .real() and .imag() methods of the complex class instead
	    // - if this is too slow, we should consider rewriting the correlator using the C complex.h header

	    // here we do the loading into the FPU inside the intrinsics (as sugested by Mark Mendell)
 	    *out_ptr = __fxcpmadd( *out_ptr, __lfps((float*) s1_val_0), __lfps(&__real__ *s2_val_0));
	    *out_ptr = __fxcxnpma( *out_ptr, __lfps((float*) s1_val_0), __lfps(&__imag__ *s2_val_0));
	    out_ptr++;

	    // note that the output buffer is assumed to be contiguous
	    // also note that I'm trying to force a add-store by using the += in the intrinsic.
	    // I don't know if this will work, since the first argument of the intrinsic may very well 
	    // just overwrite the resulting value. According to Mike Mendell this is a double add, so I removed the +=.

	    // now do the same thing for polarisation 1
	    __lfps((float*) s1_val_1);
 	    __lfps(&__real__ *s2_val_1);

	    // note that this may very well result in bogus answers. This is lots 'o hacks to get the intrinsics to compile
 	    *out_ptr = __fxcpmadd( *out_ptr, __lfps((float*) s1_val_1), __lfps(&__real__ *s2_val_1)) ;
 	    *out_ptr = __fxcxnpma( *out_ptr, __lfps((float*) s1_val_1), __lfps(&__imag__ *s2_val_1)) ;
	    out_ptr++;
#else 
	    // this is purely functional code, very expensive and slow as hell
	    outDH->addBufferElementVal(station1, station2, fchannel, 0,
				       *(s1_val_0) * *(s2_val_0));
	    outDH->addBufferElementVal(station1, station2, fchannel, 1,
				       *(s1_val_1) * *(s2_val_1));	    
#endif 	    
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
  double elapsed_time = (stoptime-starttime);
  double min_time;

  // we're selecting the highest performance figure of the nodes. Since BG/L is a hard real-time 
  // system, we expect these to be the same for each node. While debugging we're not interrested in 
  // transient effects on the nodes, so the maximum performance is a reasonable estimate of the real 
  // performance of the application.

  cmults = itsNsamples * itsNchannels * (itsNelements*itsNelements/2 + ceil(itsNelements/2.0));
  MPI_Reduce(&elapsed_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  
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
