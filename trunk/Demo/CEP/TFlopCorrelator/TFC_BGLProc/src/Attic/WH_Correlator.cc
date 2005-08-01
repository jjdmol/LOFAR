//#  filename.cc: generic correlator class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#include <lofar_config.h>
#include <stdio.h>

// General includes
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

// Application specific includes
#include <WH_Correlator.h>

#ifdef HAVE_BGL
#include <hummer_builtin.h>
#endif 

#define DO_TIMING
#define USE_BUILTIN

#define UNROLL_FACTOR 4

using namespace LOFAR;

WH_Correlator::WH_Correlator(const string& name) : 
  WorkHolder( 1, 1, name, "WH_Correlator")
{
  ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
  itsNelements      = myPS.getInt32("NRSP");                                 //myPS.getInt32("WH_Corr.stations");
  itsNsamples       = myPS.getInt32("WH_Corr.samples");
  itsNpolarisations = myPS.getInt32("polarisations");

  itsNinputs = itsNpolarisations*itsNelements;

//   itsNchannels = myPS.getInt32("WH_Corr.channels"); 
//   itsNtargets = 0; // not used?


  getDataManager().addInDataHolder(0, new DH_FIR("in", 1, myPS));
  //  getDataManager().addInDataHolder(0, new DH_CorrCube("in", 1));
  getDataManager().addOutDataHolder(0, new DH_Vis("out", 1, myPS));

  t_start.tv_sec = 0;
  t_start.tv_usec = 0;

  bandwidth=0.0;
  agg_bandwidth=0.0;

  corr_perf=0.0;

}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct (const string& name)
{
  return new WH_Correlator(name);
}

WH_Correlator* WH_Correlator::make (const string& name) {
  return new WH_Correlator(name);
}

void WH_Correlator::preprocess() {
  ASSERTSTR(itsNinputs == ELEMENTS, "configuration from file does not match defined configuration");
  ASSERTSTR(itsNsamples == SAMPLES, "configuration from file does not match defined configuration");

  ASSERTSTR(static_cast<DH_FIR*>(getDataManager().getInHolder(0))->getBufferSize() == ELEMENTS*SAMPLES, "InHolder size not equal to defined size");
//   ASSERTSTR(static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBufSize() == ELEMENTS*SAMPLES, "InHolder size not equal to defined size");
  ASSERTSTR(static_cast<DH_Vis*>(getDataManager().getOutHolder(0))->getBufSize() == ELEMENTS*ELEMENTS, "OutHolder size not equal to defined size");

  // prevent stupid mistakes in the future by assuming we can easily change the unroll factor
  ASSERTSTR(UNROLL_FACTOR == 4, "Code is normally only unrolled by a factor of 4, make sure this is really what you want!");
}

void WH_Correlator::process() {
  double starttime, stoptime, cmults;
  const short ar_block = itsNinputs / UNROLL_FACTOR;

  DH_FIR *inDH  = (DH_FIR*)(getDataManager().getInHolder(0));
//   DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  // reset integrator.
  memset(outDH->getBuffer(), 0, outDH->getBufSize()*sizeof(DH_Vis::BufferType));


#ifdef DO_TIMING
  starttime = timer();
#endif


  __complex__ double reg_x_0, reg_x_1, reg_x_2, reg_x_3;
  __complex__ double reg_y_0, reg_y_1, reg_y_2, reg_y_3;
  
  __complex__ double outData[ELEMENTS*ELEMENTS*4];  // large enough; can be made smaller; 4 pol 
  __complex__ double *outptr;

#ifdef HAVE_BGL
  __alignx(16, outptr);
  //  __alignx(8 , in_buffer);
#endif

  int A;
  for (int time=0; time< SAMPLES; time++) {
    outptr = outData;  // point to start of out buffer
    // the output data will be sequentially written in this buffer
    // after completion of the outer (time) loop data
    // will be copied correctly into the output DH.

    //loop over the Y dimension
    for (int B=0; B< ELEMENTS; B+=2) {
      // addressing:   getBufferElement(    channel, station, time, pol)
      reg_B0_X = inDH->getBufferElement(int channel, B,       time, 0);     
      reg_B0_Y = inDH->getBufferElement(int channel, B,       time, 1);     
      reg_B1_X = inDH->getBufferElement(int channel, B+1,     time, 0);     
      reg_B1_Y = inDH->getBufferElement(int channel, B+1,     time, 1);     
      // now loop over the A dimension
      for (A=0; A<ELEMENTS-2-B; A+=2) {
	// now correlate stations A,A+1,B,B+1 in both polarisations
	//cout << A   << " - " << B   << endl;
	//cout << A   << " - " << B+1 << endl;
	//cout << A+1 << " - " << B   << endl;
	//cout << A+1 << " - " << B+1 << endl;
 	// load A inputs into registers
	reg_A0_X = inDH->getBufferElement(int channel, A  , time, 0);     
	reg_A0_Y = inDH->getBufferElement(int channel, A  , time, 1);     
	reg_A1_X = inDH->getBufferElement(int channel, A+1, time, 0);     
	reg_A1_Y = inDH->getBufferElement(int channel, A+1, time, 1);      
	// calculate all correlations; 
	// todo: prefetch new A dimesnsions on the way
	*(outptr++) += reg_A0_X * ~reg_B0_X;
	*(outptr++) += reg_A0_X * ~reg_B0_Y;
	*(outptr++) += reg_A0_X * ~reg_B1_X;
	*(outptr++) += reg_A0_X * ~reg_B1_Y;
	
	*(outptr++) += reg_A0_Y * ~reg_B0_X;
	*(outptr++) += reg_A0_Y * ~reg_B0_Y;
	*(outptr++) += reg_A0_Y * ~reg_B1_X;
	*(outptr++) += reg_A0_Y * ~reg_B1_Y;
	
	*(outptr++) += reg_A1_X * ~reg_B0_X;
	*(outptr++) += reg_A1_X * ~reg_B0_Y;
	*(outptr++) += reg_A1_X * ~reg_B1_X;
	*(outptr++) += reg_A1_X * ~reg_B1_Y;
	
	*(outptr++) += reg_A1_Y * ~reg_B0_X;
	*(outptr++) += reg_A1_Y * ~reg_B0_Y;
	*(outptr++) += reg_A1_Y * ~reg_B1_X;
	*(outptr++) += reg_A1_Y * ~reg_B1_Y;
	
      }
    }
    // done all sqaures
    // now correlate the last triangle;
    //cout << A   << " - " << B   << endl;
    //cout << A   << " - " << B+1 << endl;
    //cout << A+1 << " - " << B   << endl;
    reg_A0_X = getBufferElement(int channel, A  , time, 0);     
    reg_A0_Y = getBufferElement(int channel, A  , time, 1);     
    reg_A1_X = getBufferElement(int channel, A+1, time, 0);     
    reg_A1_Y = getBufferElement(int channel, A+1, time, 1);     
    // calculate all correlations in the triangle; 
    // todo: prefetch new A dimesnsions on the way
    *(outptr++) += reg_A0_X * ~reg_B0_X;
    *(outptr++) += reg_A0_X * ~reg_B0_Y;
    *(outptr++) += reg_A0_X * ~reg_B1_X;
    *(outptr++) += reg_A0_X * ~reg_B1_Y;
    
    *(outptr++) += reg_A0_Y * ~reg_B0_X;
    *(outptr++) += reg_A0_Y * ~reg_B0_Y;
    *(outptr++) += reg_A0_Y * ~reg_B1_X;
    *(outptr++) += reg_A0_Y * ~reg_B1_Y;
   
    *(outptr++) += reg_A1_X * ~reg_B0_X;
    *(outptr++) += reg_A1_X * ~reg_B0_Y;
    
    *(outptr++) += reg_A1_Y * ~reg_B0_X;
    *(outptr++) += reg_A1_Y * ~reg_B0_Y;
     
  }

  // todo: write the Outptr data into DH_Vis.

#ifdef DO_TIMING
  stoptime = timer();
#endif
 
}

void WH_Correlator::dump() const{
  for (int x = 0; x < ELEMENTS; x++) {
    for (int y = 0; y <= x; y++) {
      // show transposed correlation matrix, this looks more natural.
      cout << out_buffer[y][x] << "  ";
    }
    cout << endl;
  }
}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
