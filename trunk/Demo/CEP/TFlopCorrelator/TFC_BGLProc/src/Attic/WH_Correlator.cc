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
  itsNelements      = myPS.getInt32("Input.NRSP");
  itsNsamples       = myPS.getInt32("Input.NSamplesToDH");
  itsNpolarisations = myPS.getInt32("Input.NPolarisations");

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
  int dhSize = (static_cast<DH_FIR*>(getDataManager().getInHolder(0)))->getBufferSize();
  int mySize = itsNelements*itsNpolarisations*itsNsamples;

  ASSERTSTR(dhSize == mySize, 
	    "InHolder size not equal to workholder size ("<<dhSize<<" != "<<mySize<<")");

  ASSERTSTR((static_cast<DH_Vis*>(getDataManager().getOutHolder(0)))->getBufSize() == 
	    itsNpolarisations*itsNpolarisations*itsNelements*(itsNelements+1)/2, 
	    "OutHolder size not equal to workholder size");

  // prevent stupid mistakes in the future by assuming we can easily change the unroll factor
  ASSERTSTR(UNROLL_FACTOR == 4, "Code is normally only unrolled by a factor of 4, make sure this is really what you want!");
}

void WH_Correlator::process() {
  double starttime, stoptime /*, cmults*/;
  // const short ar_block = itsNinputs / UNROLL_FACTOR;
  // dummy channel parameter; not used in the first correlator version
  const int channel = 0;

  DH_FIR *inDH  = (DH_FIR*)(getDataManager().getInHolder(0));
//   DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  // reset integrator.
  memset(outDH->getBuffer(), 0, outDH->getBufSize()*sizeof(DH_Vis::BufferType));


#ifdef DO_TIMING
  starttime = timer();
#endif
  
  __complex__ double reg_A0_X, reg_A0_Y, reg_A1_X, reg_A1_Y;
  __complex__ double reg_B0_X, reg_B0_Y, reg_B1_X, reg_B1_Y;
    
  
//   __complex__ double outData[ELEMENTS*ELEMENTS*4];  // large enough; can be made smaller; 4 pol 
  __complex__ double outData[itsNelements*itsNelements*itsNpolarisations*itsNpolarisations];  // large enough; 
  __complex__ double *outptr0, *outptr1;

  /// initialize output buffer
  memset(&outData[0], 
	 0,
	 itsNelements*itsNelements*itsNpolarisations*itsNpolarisations*sizeof(__complex__ double));
  

#ifdef HAVE_BGL
  __alignx(16, outptr0);
  __alignx(16, outptr1);
  //  __alignx(8 , in_buffer);
#endif

  int B = 0;

  for (int time=0; time< itsNsamples; time++) {
    // refer to addressing in DH_Vis.h::getBufferElement() 
    // loop over the vertical(rows) dimension in the correlation matrix
    for (int A=0; A < itsNelements; A+=2) {
      // addressing:   getBufferElement(    channel, station, time, pol)
#ifdef HAVE_BGL
      reg_A0_X = ficast(inDH->getBufferElement(channel, A,       time, 0));     
      reg_A0_Y = ficast(inDH->getBufferElement(channel, A,       time, 1));     
      reg_A1_X = ficast(inDH->getBufferElement(channel, A+1,     time, 0));     
      reg_A1_Y = ficast(inDH->getBufferElement(channel, A+1,     time, 1));     
#else
      reg_A0_X = inDH->getBufferElement(channel, A,       time, 0);     
      reg_A0_Y = inDH->getBufferElement(channel, A,       time, 1);     
      reg_A1_X = inDH->getBufferElement(channel, A+1,     time, 0);     
      reg_A1_Y = inDH->getBufferElement(channel, A+1,     time, 1);     
#endif
      //
      // get pointers to the two rows we calculate in the next loop
      
      outptr0 = &outData[outDH->getBufferOffset(A  , 0, 0)]; // first row
      outptr1 = &outData[outDH->getBufferOffset(A+1, 0, 0)]; // second row
      // now loop over the B dimension
      for (B=0; B<A; B+=2) {
	// these are the full squares
	// now correlate stations B,B+1,A,A+1 in both polarisations
// 	cout << B   << " - " << A   << endl;
// 	cout << B   << " - " << A+1 << endl;
// 	cout << B+1 << " - " << A   << endl;
// 	cout << B+1 << " - " << A+1 << endl;

 	// load B inputs into registers
#ifdef HAVE_BGL
	reg_B0_X = ficast(inDH->getBufferElement(channel, B  , time, 0));
	reg_B0_Y = ficast(inDH->getBufferElement(channel, B  , time, 1));     
	reg_B1_X = ficast(inDH->getBufferElement(channel, B+1, time, 0));     
	reg_B1_Y = ficast(inDH->getBufferElement(channel, B+1, time, 1));      
#else
	reg_B0_X = inDH->getBufferElement(channel, B  , time, 0);
	reg_B0_Y = inDH->getBufferElement(channel, B  , time, 1);     
	reg_B1_X = inDH->getBufferElement(channel, B+1, time, 0);     
	reg_B1_Y = inDH->getBufferElement(channel, B+1, time, 1);      
#endif
	// calculate all correlations; 
	// todo: prefetch new B dimesnsions on the way
	DBGASSERT(outptr0 == &outData[outDH->getBufferOffset(A,B,0)]);
	*(outptr0++) += reg_A0_X * ~reg_B0_X;
	*(outptr0++) += reg_A0_X * ~reg_B0_Y;
	*(outptr0++) += reg_A0_Y * ~reg_B0_X;
	*(outptr0++) += reg_A0_Y * ~reg_B0_Y;
	
	DBGASSERT(outptr0 == &outData[outDH->getBufferOffset(A,B+1,0)]);
	*(outptr0++) += reg_A0_X * ~reg_B1_X;
	*(outptr0++) += reg_A0_X * ~reg_B1_Y;
	*(outptr0++) += reg_A0_Y * ~reg_B1_X;
	*(outptr0++) += reg_A0_Y * ~reg_B1_Y;
	
	DBGASSERT(outptr1 == &outData[outDH->getBufferOffset(A+1,B,0)]);
	*(outptr1++) += reg_A1_X * ~reg_B0_X;
	*(outptr1++) += reg_A1_X * ~reg_B0_Y;
	*(outptr1++) += reg_A1_Y * ~reg_B0_X;
	*(outptr1++) += reg_A1_Y * ~reg_B0_Y;
	
	DBGASSERT(outptr1 == &outData[outDH->getBufferOffset(A+1,B+1,0)]);
	*(outptr1++) += reg_A1_X * ~reg_B1_X;
	*(outptr1++) += reg_A1_X * ~reg_B1_Y;
	*(outptr1++) += reg_A1_Y * ~reg_B1_X;
	*(outptr1++) += reg_A1_Y * ~reg_B1_Y;
      }
    
      // done all sqaures
      // now correlate the last triangle;
//       cout << B   << " - " << A   << endl;
//       cout << B   << " - " << A+1 << endl;
//       cout << B+1 << " - " << A+1 << endl;

#ifdef HAVE_BGL
      reg_B0_X = ficast(inDH->getBufferElement(channel, B  , time, 0));     
      reg_B0_Y = ficast(inDH->getBufferElement(channel, B  , time, 1));     
      reg_B1_X = ficast(inDH->getBufferElement(channel, B+1, time, 0));     
      reg_B1_Y = ficast(inDH->getBufferElement(channel, B+1, time, 1));     
#else
      reg_B0_X = inDH->getBufferElement(channel, B  , time, 0);     
      reg_B0_Y = inDH->getBufferElement(channel, B  , time, 1);     
      reg_B1_X = inDH->getBufferElement(channel, B+1, time, 0);     
      reg_B1_Y = inDH->getBufferElement(channel, B+1, time, 1);     

      DBGASSERTSTR(reg_A0_X == reg_B0_X, "0_X not equal in loop " << A << " " << B << " " << time << " " << channel);
      DBGASSERTSTR(reg_A0_Y == reg_B0_Y, "0_Y not equal in loop " << A << " " << B << " " << time << " " << channel);
      DBGASSERTSTR(reg_A1_X == reg_B1_X, "1_X not equal in loop " << A << " " << B << " " << time << " " << channel);
      DBGASSERTSTR(reg_A1_Y == reg_B1_Y, "1_Y not equal in loop " << A << " " << B << " " << time << " " << channel);
#endif
      // calculate all correlations in the triangle; 
      // todo: prefetch new B dimesnsions on the way
      DBGASSERT(outptr0 == &outData[outDH->getBufferOffset(A,B,0)]);
      *(outptr0++) += reg_A0_X * ~reg_B0_X;
      *(outptr0++) += reg_A0_X * ~reg_B0_Y;
      *(outptr0++) += reg_A0_Y * ~reg_B0_X;
      *(outptr0++) += reg_A0_Y * ~reg_B0_Y;
      
      DBGASSERT(outptr1 == &outData[outDH->getBufferOffset(A+1,B,0)]);
      *(outptr1++) += reg_A1_X * ~reg_B0_X;
      *(outptr1++) += reg_A1_X * ~reg_B0_Y;  
      *(outptr1++) += reg_A1_Y * ~reg_B0_X;
      *(outptr1++) += reg_A1_Y * ~reg_B0_Y;

      DBGASSERT(outptr1 == &outData[outDH->getBufferOffset(A+1,B+1,0)]);
      *(outptr1++) += reg_A1_X * ~reg_B1_X;
      *(outptr1++) += reg_A1_X * ~reg_B1_Y;
      *(outptr1++) += reg_A1_Y * ~reg_B1_X;
      *(outptr1++) += reg_A1_Y * ~reg_B1_Y;        
    }
  }

  // todo: write the Outptr data into DH_Vis.
  // since the addressing in outptr is exactly the same as in DH_Vis, we can now 
  // copy and convert to float element for element

  DH_Vis::BufferType *dhptr = outDH->getBufferElement(0,0,0);
  outptr0 = &outData[0];
  int loopsize = outDH->getBufferOffset(itsNelements-1, itsNelements-1, 4) - outDH->getBufferOffset(0,0,0);

  for (int i=0; i < loopsize; i++) {
    dhptr[i] = outptr0[i]; 
  }

#ifdef DO_TIMING
  stoptime = timer();
#endif
 
}

void WH_Correlator::dump() const{
  DH_Vis *outDH = static_cast<DH_Vis*>(getDataManager().getOutHolder(0));

  for (short pol = 0; pol < itsNpolarisations*itsNpolarisations; pol++) { 
    cout << "POL : " << pol << endl;
    for (short x = 0; x < itsNelements; x++) {
      for (short y = 0; y <= x; y++) {
      

	// show transposed correlation matrix, this looks more natural.
	cout << *outDH->getBufferElement(x, y, pol) << " ";
      }
      cout << endl;
    } 
    cout << endl;
  } 
}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
