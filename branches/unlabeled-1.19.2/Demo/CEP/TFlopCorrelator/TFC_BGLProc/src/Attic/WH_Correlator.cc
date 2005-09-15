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
  void _correlate_2x2(const fcomplex *S0, const fcomplex *S1, const fcomplex *S2, const fcomplex *S3, fcomplex *out, const float samples);
//   void _correlate_2x3(const fcomplex *samples, dcomplex *out);
}

WH_Correlator::WH_Correlator(const string& name, int nfilters, int nchannels):
  WorkHolder(nfilters, nchannels, name, "WH_Correlator")
{
  ACC::APS::ParameterSet myPS("TFlopCorrelator.cfg");

  itsNfilters = myPS.getInt32("PPF.NrFilters");
  itsNsamples = myPS.getInt32("PPF.NrStationSamples");
  itsNelements = myPS.getInt32("PPF.NrStations");
  itsNpolarisations = myPS.getInt32("PPF.NrPolarizations");
  itsNchannels = myPS.getInt32("PPF.NrSubChannels") / myPS.getInt32("PPF.NrCorrelatorsPerFilter");
    

  ASSERTSTR(itsNelements      == NR_STATIONS, "Configuration doesn't match paramters: NrStations");
  ASSERTSTR(itsNpolarisations == NR_POLARIZATIONS, "Configuration doesn't match paramters: NrPolarizations");
  ASSERTSTR(itsNchannels      == NR_CHANNELS_PER_CORRELATOR, "Configuration doesn't match paramters: NrChannels");
  ASSERTSTR(itsNsamples       == NR_STATION_SAMPLES, "Configuration doesn't match paramters: NrSamples");

  for (int i = 0; i < itsNfilters; i++) {
    char str[50];
    snprintf(str, 50, "input_%d_of_%d", i, itsNfilters);
    getDataManager().addInDataHolder(0, new DH_CorrCube(str, 0));
  }
  for (int i = 0; i < itsNchannels; i++) {
    char str[50];
    snprintf(str, 50, "output_%d_of_%d", i, itsNchannels);
    getDataManager().addOutDataHolder(i, new DH_Vis(str, 0, myPS));
  }
}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct(const string& name, int nfilters, int nchannels) {
  return new WH_Correlator(name, nfilters, nchannels);
}

WH_Correlator* WH_Correlator::make(const string& name) {
  return new WH_Correlator(name, itsNfilters, itsNchannels);
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
  // Generic C++ version.. Should be considered a reference implementation.
  

#else
  // Blue Gene/L assembler version. 
  // Divide the correlation matrix into blocks of 2x2. 
  // Correlate the entire block (all time samples) before 
  // going on to the next block.
  DH_CorrCube::BufferType *inputs[itsNfilters];

  // define a type containing the input from a single station
  typedef fcomplex stationInputType[NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS];

  for (int i = 0; i < itsNfilters; i++) {
    inputs[i] = static_cast<DH_CorrCube*>(getDataManager().getInHolder(i))->getBuffer();
  }
  
  DH_Vis::BufferType *outputs[NR_CHANNELS_PER_CORRELATOR];
  for (int i = 0; i < NR_CHANNELS_PER_CORRELATOR; i++) {
    outputs[i] = (DH_Vis::BufferType*) static_cast<DH_Vis*>(getDataManager().getOutHolder(i))->getBuffer();
  }
  
  for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch++) {

    for (int x = 0; x < itsNelements; x += 2 ){
      stationInputType *S0 = (stationInputType*) static_cast<DH_CorrCube*>(getDataManager().getInHolder( ch / ( NR_STATIONS/itsNfilters ) ))->getBufferElement(ch, x, 0, 0);
      stationInputType *S1 = (stationInputType*) static_cast<DH_CorrCube*>(getDataManager().getInHolder( ch / ( NR_STATIONS/itsNfilters ) ))->getBufferElement(ch, x+1, 0, 0);
      for (int y = 0; y <= x; y += 2) { 
	stationInputType *S2 = (stationInputType*) static_cast<DH_CorrCube*>(getDataManager().getInHolder( ch / ( NR_STATIONS/itsNfilters ) ))->getBufferElement(ch, y, 0, 0);
	stationInputType *S3 = (stationInputType*) static_cast<DH_CorrCube*>(getDataManager().getInHolder( ch / ( NR_STATIONS/itsNfilters ) ))->getBufferElement(ch, y+1, 0, 0);

	_correlate_2x2(&(*S0)[0][0], &(*S1)[0][0], &(*S2)[0][0], &(*S3)[0][0], outputs[ch], NR_SAMPLES_PER_INTEGRATION);
      }
    }
  }

#endif  
  timer.stop();
}

void WH_Correlator::postprocess() {
}

void WH_Correlator::dump() const {
}
