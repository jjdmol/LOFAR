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

//    cout << "(" << TH_MPI::getCurrentRank() <<") " << bandwidth/(1024.0*1024.0) << " Mbit/sec" << endl;
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
  // todo: speed up by creating 0-filled array in C'tor and memcpy in one stroke.
  DH_Vis::BufferType *zero = new DH_Vis::BufferType(0,0);
    for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
      for (int   station1 = 0; station1 < itsNelements; station1++) {
	for (int station2 = 0; station2 <= station1;    station2++) {
	  for (int polarisation = 0; polarisation < itsNpolarisations; polarisation++) {
	    outDH->setBufferElement(station1, station2, fchannel, polarisation, zero);
	  }
	}
      }
    }

#ifdef DO_TIMING
  starttime = timer();
#endif

  // calculate the correlations and add to output DataHolder.
  DH_Vis::BufferType s1_val, s2_val;

  for (int sample = 0; sample < itsNsamples; sample++) {
#ifdef HAVE_MPE
    MPE_Log_event(1, sample, "correlating"); 
#endif
    for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
      for (int   station1 = 0; station1 < itsNelements; station1++) {
	for (int station2 = 0; station2 <= station1;    station2++) {

	  for (int polarisation = 0; polarisation < itsNpolarisations; polarisation++) {
	    // todo: use copy-free multiplication
	    // todo: remove inner loop getBufferElement calls; consecutive adressing
	    // todo: do short-> float conversion only once
	  
	    // convert complex<short> to complex<float>
	    s1_val = DH_Vis::BufferType((inDH->getBufferElement(fchannel, sample, station1, polarisation))->real(),
					(inDH->getBufferElement(fchannel, sample, station1, polarisation)->imag()));
	    s2_val = DH_Vis::BufferType((inDH->getBufferElement(fchannel, sample, station2, polarisation))->real(),
					(inDH->getBufferElement(fchannel, sample, station2, polarisation)->imag()));
	    
	    outDH->addBufferElementVal(station1, station2, fchannel, polarisation,
				       s1_val * s2_val
				       );

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
  double max_cmults;

  // we're selecting the highest performance figure of the nodes. Since BG/L is a hard real-time 
  // system, we expect these to be the same for each node. While debugging we're not interrested in 
  // transient effects on the nodes, so the maximum performance is a reasonable estimate of the real 
  // performance of the application.
  cmults = itsNsamples * itsNchannels * (itsNelements*itsNelements/2 + ceil(itsNelements/2.0));
  MPI_Reduce(&cmults, &max_cmults, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  
  if ((TH_MPI::getCurrentRank() == 0) && (t_start.tv_sec != 0) && (t_start.tv_usec != 0)) {
    
    cout << 10e-6*max_cmults/(stoptime-starttime) << " Mcprod/sec" << endl;
    //   cout << itsNsamples << " " << itsNelements << " " << 10e-6*cmults/(stoptime-starttime) << endl;
  }

#endif
  gettimeofday(&t_start, NULL);
#endif
}				     

void WH_Correlator::dump() {

}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
