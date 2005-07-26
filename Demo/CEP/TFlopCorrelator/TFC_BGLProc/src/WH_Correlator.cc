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
#include <TFC_Interface/DH_Vis.h>
#include <TFC_Interface/DH_CorrCube.h>
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
  itsNelements = myPS.getInt32("WH_Corr.stations");
  itsNsamples  = myPS.getInt32("WH_Corr.samples");
  itsNchannels = myPS.getInt32("WH_Corr.channels"); 
  itsNtargets = 0; // not used?

  getDataManager().addInDataHolder(0, new DH_CorrCube("in", 1));
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

  ASSERTSTR(static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBufSize() == ELEMENTS*SAMPLES, "InHolder size not equal to defined size");
  ASSERTSTR(static_cast<DH_Vis*>(getDataManager().getOutHolder(0))->getBufSize() == ELEMENTS*ELEMENTS, "OutHolder size not equal to defined size");

  // prevent stupid mistakes in the future by assuming we can easily change the unroll factor
  ASSERTSTR(UNROLL_FACTOR == 4, "Code is normally only unrolled by a factor of 4, make sure this is really what you want!");
}

void WH_Correlator::process() {
  double starttime, stoptime, cmults;
  const short ar_block = itsNelements / UNROLL_FACTOR;

  DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  in_ptr = &(in_buffer[0][0]);
  out_ptr = &(out_buffer[0][0]);

  // reset integrator.
  memset(outDH->getBuffer(), 0, outDH->getBufSize()*sizeof(DH_Vis::BufferType));

  // fill the input buffer
  // could this be done without a memcpy?
  memcpy(&in_buffer, inDH->getBuffer(), inDH->getBufSize()*sizeof(DH_CorrCube::BufferType));

#ifdef DO_TIMING
  starttime = timer();
#endif

#ifdef HAVE_BGL
  __alignx(16, out_buffer);
  __alignx(8 , in_buffer);
#endif

  // The actual correlator loop
  // Note that we don't make special considerations for polarisations X or Y
  // We consider these completely seperate elements, thus removing an 
  // extra dimension in the input and output data structure
 
  // Also note that this algorithm calculates slightly more correlation products than 
  // strictly necessary. For now we assume that the extra overhead introduced by 
  // exactly calculating half the matrix is more than what we calculate extra now.
  // This may be optimized in a later version.

  for (int i = 0; i < itsNsamples; i++) {
    for (int y = 0; y < itsNelements; y+=4) { 

      // prefetch a block of values to register
      __complex__ double reg_x_0 = static_cast<__complex__ double>(in_buffer[0][i]);
      __complex__ double reg_x_1 = static_cast<__complex__ double>(in_buffer[1][i]);
      __complex__ double reg_x_2 = static_cast<__complex__ double>(in_buffer[2][i]);
      __complex__ double reg_x_3 = static_cast<__complex__ double>(in_buffer[3][i]);

      __complex__ double reg_y_0 = static_cast<__complex__ double>(in_buffer[y][i]);
      __complex__ double reg_y_1 = static_cast<__complex__ double>(in_buffer[y+1][i]);
      __complex__ double reg_y_2 = static_cast<__complex__ double>(in_buffer[y+2][i]);
      __complex__ double reg_y_3 = static_cast<__complex__ double>(in_buffer[y+3][i]);

      for (int x = 0; x <= y; x+=4) {

	out_buffer[x+0][y+0] += reg_x_0 * ~reg_y_0;
	out_buffer[x+0][y+1] += reg_x_0 * ~reg_y_1;
	out_buffer[x+0][y+2] += reg_x_0 * ~reg_y_2;
	out_buffer[x+0][y+3] += reg_x_0 * ~reg_y_3;

	reg_x_0 = in_buffer[x+4][i];

	out_buffer[x+1][y+0] += reg_x_1 * ~reg_y_0;
	out_buffer[x+1][y+1] += reg_x_1 * ~reg_y_1;
	out_buffer[x+1][y+2] += reg_x_1 * ~reg_y_2;
	out_buffer[x+1][y+3] += reg_x_1 * ~reg_y_3;
	
	reg_x_1 = in_buffer[x+5][i];

	out_buffer[x+2][y+0] += reg_x_2 * ~reg_y_0;
	out_buffer[x+2][y+1] += reg_x_2 * ~reg_y_1;
	out_buffer[x+2][y+2] += reg_x_2 * ~reg_y_2;
	out_buffer[x+2][y+3] += reg_x_2 * ~reg_y_3;

	reg_x_2 = in_buffer[x+6][i];

	out_buffer[x+3][y+0] += reg_x_3 * ~reg_y_0;
	out_buffer[x+3][y+1] += reg_x_3 * ~reg_y_1;
	out_buffer[x+3][y+2] += reg_x_3 * ~reg_y_2;
	out_buffer[x+3][y+3] += reg_x_3 * ~reg_y_3;

	reg_x_3 = in_buffer[x+7][i];
      }
    }
  }

  // this is allowed since we're sure the outDH is equal in size to the 
  // out_buffer (the assert didn't fail in the preprocess method)
  outDH->setBuffer(out_ptr);
  //   memcpy(outDH->getBuffer(), out_buffer, ELEMENTS*ELEMENTS*sizeof(DH_Vis::BufferType));

#ifdef DO_TIMING
  stoptime = timer();
#endif
 
}

void WH_Correlator::dump() {
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
