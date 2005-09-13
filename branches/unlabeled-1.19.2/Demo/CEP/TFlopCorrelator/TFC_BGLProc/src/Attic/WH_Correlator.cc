//#  WH_Correlator.cc:
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# includes
#include <APS/ParameterSet.h>
#include <WH_Correlator.h>

#include <hummer_builtin.h>

using namespace LOFAR;

extern "C"
{
  void _correlator(const fcomplex *samples, dcomplex *out);
}

WH_Correlator::WH_Correlator(const string& name, int nchannels):
  WorkHolder(1, nchannels, name, "WH_Correlator")
{
  ACC::APS::ParameterSet myPS("TFlopCorrelator.cfg");

  itsNsamples = myPS.getInt32("PPF.NrStationSamples");
  itsNelements = myPS.getInt32("PPF.NrStations");
  itsNpolarisations = myPS.getInt32("PPF.NrPolarizations");
  itsNchannels = myPS.getInt32("PPF.NrSubChannels") / myPS.getInt32("PPF.NrCorrelatorsPerFilter");
    

  ASSERTSTR(itsNelements      == NR_STATIONS, "Configuration doesn't match paramters: NrStations");
  ASSERTSTR(itsNpolarisations == NR_POLARIZATIONS, "Configuration doesn't match paramters: NrPolarizations");
  ASSERTSTR(itsNchannels      == NR_CHANNELS_PER_CORRELATOR, "Configuration doesn't match paramters: NrChannels");


  getDataManager().addInDataHolder(0, new DH_CorrCube("input", 0));
  for (int i = 0; i < itsNchannels; i++) {
    char str[50];
    snprintf(str, 50, "output_%d_of_%d", i, itsNchannels);
    getDataManager().addOutDataHolder(i, new DH_Vis(str, 0, myPS));
  }
}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct(const string& name, int nchannels) {
  return new WH_Correlator(name, nchannels);
}

WH_Correlator* WH_Correlator::make(const string& name) {
  return new WH_Correlator(name, itsNchannels);
}

void WH_Correlator::preprocess() { 
}

void WH_Correlator::process() {

  DH_CorrCube* inHolderPtr = static_cast<DH_CorrCube*>(getDataManager().getInHolder(0));
  DH_Vis*      outHolderPtr = static_cast<DH_Vis*>(getDataManager().getOutHolder(0));
  DH_Vis::BufferType* outPtr = 0;

  
  
  static NSTimer timer("WH_Correlator::process()", true);

  timer.start();
#if 0
  dcomplex r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
  dcomplex r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
  dcomplex r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
  dcomplex r30, r31;

  __alignx(8, inHolderPtr->getBuffer());
  __alignx(16, outHolderPtr->getBuffer());

  for (int A = 0; A < itsNelements; A++) {
    for (int B = 0; B <= A; B++) {

      r0 = *inHolderPtr->getBufferElement(0, A, 0, 0);
      r1 = *inHolderPtr->getBufferElement(0, A, 0, 1);

      r2 = *inHolderPtr->getBufferElement(1, A, 0, 0);
      r3 = *inHolderPtr->getBufferElement(1, A, 0, 1);

      r4 = *inHolderPtr->getBufferElement(2, A, 0, 0);
      r5 = *inHolderPtr->getBufferElement(2, A, 0, 0);
      
      r10 = *inHolderPtr->getBufferElement(0, B, 0, 0);
      r11 = *inHolderPtr->getBufferElement(0, B, 0, 1);
      
      r12 = *inHolderPtr->getBufferElement(1, B, 0, 0);
      r13 = *inHolderPtr->getBufferElement(1, B, 0, 1);

      r14 = *inHolderPtr->getBufferElement(2, B, 0, 0);
      r15 = *inHolderPtr->getBufferElement(2, B, 0, 1);

      for (int time = 1; time <= itsNsamples; time++) {

	// now loop over all channels to determine cross products
	// this is unrolled for clarity and speed

	// channel 0
	r20 = r0 * ~r10; // XX
	r21 = r0 * ~r11; // XY
	r22 = r1 * ~r10; // YX
	r23 = r1 * ~r11; // YY

	r0 = *inHolderPtr->getBufferElement(0, A, time, 0); // time may overflow?
	r10 = *inHolderPtr->getBufferElement(0, B, time, 0);
	r1 = *inHolderPtr->getBufferElement(0, A, time, 1);
	r11 = *inHolderPtr->getBufferElement(0, B, time, 1);
	
	// channel 1
	r24 = r2 * ~r12; // XX
	r25 = r2 * ~r13; // XY
	r26 = r3 * ~r12; // YX
	r27 = r3 * ~r13; // YY

	r2 = *inHolderPtr->getBufferElement(1, A, time, 0);
	r12 = *inHolderPtr->getBufferElement(1, B, time, 0);
	r3 = *inHolderPtr->getBufferElement(1, A, time, 1);
	r13 = *inHolderPtr->getBufferElement(1, B, time, 1);

	// channel 2
	r28 = r4 * ~r14; // XX
	r29 = r4 * ~r15; // XY
	r30 = r5 * ~r14; // YX
	r31 = r5 * ~r15; // YY

	r4 = *inHolderPtr->getBufferElement(1, A, time, 0);
	r14 = *inHolderPtr->getBufferElement(1, B, time, 0);
	r5 = *inHolderPtr->getBufferElement(1, A, time, 1);
	r15 = *inHolderPtr->getBufferElement(1, B, time, 1);

      }
      // store accumulated results
      
    }
  }
#else
//   _correlator(inHolderPtr->getBuffer(), outHolderPtr->getBuffer());
#endif  
  timer.stop();
}

void WH_Correlator::postprocess() {
}

void WH_Correlator::dump() const {
}
