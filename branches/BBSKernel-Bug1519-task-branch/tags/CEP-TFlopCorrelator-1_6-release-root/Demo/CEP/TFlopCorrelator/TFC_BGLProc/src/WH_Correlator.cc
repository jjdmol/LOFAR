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
#include <TFC_Interface/TFC_Config.h>
#include <TFC_Interface/DH_Vis.h>
#include <TFC_Interface/DH_CorrCube.h>
#include <WH_Correlator.h>
#include <Correlator.h>

#if 0 || !defined HAVE_BGL
#define C_IMPLEMENTATION
#endif

using namespace LOFAR;


WH_Correlator::WH_Correlator(const string &name, const ACC::APS::ParameterSet& ps)
:
  WorkHolder(NR_PPF_PER_COMPUTE_CELL, 1, name, "WH_Correlator"),
  itsPS(ps)
{
  itsNfilters = itsPS.getInt32("BGLProc.NFiltersPerComputeCell");
  itsNsamples = itsPS.getInt32("Data.NSamplesToIntegrate");
  itsNelements = itsPS.getInt32("FakeData.NStations");
  itsNpolarisations = itsPS.getInt32("Data.NPolarisations");
  itsNchannels = itsPS.getInt32("Data.NChannels") / itsPS.getInt32("BGLProc.NCorrelatorsPerComputeCell");
    

  ASSERTSTR(itsNelements      == NR_STATIONS, "Configuration doesn't match parameter: NrStations");
  ASSERTSTR(itsNpolarisations == NR_POLARIZATIONS, "Configuration doesn't match parameter: NrPolarizations");
  ASSERTSTR(itsNchannels      == NR_CHANNELS_PER_CORRELATOR, "Configuration doesn't match parameter: NrChannels");
  ASSERTSTR(itsNsamples       == NR_STATION_SAMPLES, "Configuration doesn't match parameter: NrSamples");
  ASSERTSTR(itsNfilters       == NR_PPF_PER_COMPUTE_CELL, "Configuration doesn't match parameter: NrFilters");
  ASSERTSTR(NR_PPF_PER_COMPUTE_CELL == 1 || NR_PPF_PER_COMPUTE_CELL == 2, "Only 1 or 2 inputs implemented in the correlator");
  //totalInputSize = 0;

  for (int i = 0; i < NR_PPF_PER_COMPUTE_CELL; i++) {
    char str[50];
    snprintf(str, 50, "input_%d_of_%d", i, NR_PPF_PER_COMPUTE_CELL);
    getDataManager().addInDataHolder(i, new DH_CorrCube(str, 0, itsPS));
    //totalInputSize += static_cast<DH_CorrCube*>(getDataManager().getInHolder(i))->getBufSize();
  }

//   for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch ++) {
//     char str[50];
//     snprintf(str, 50, "output_%d_of_%d", ch, NR_CHANNELS_PER_CORRELATOR);
//     getDataManager().addOutDataHolder(ch, new DH_Vis(str, 0, itsPS));
//   }
  getDataManager().addOutDataHolder(0, new DH_Vis("output", 0, itsPS));

}

WH_Correlator::~WH_Correlator()
{
}

WorkHolder* WH_Correlator::construct(const string& name)
{
  return new WH_Correlator(name, itsPS);
}

WH_Correlator* WH_Correlator::make(const string& name)
{
  return new WH_Correlator(name, itsPS);
}

void WH_Correlator::preprocess()
{ 
}

void WH_Correlator::process()
{
  static NSTimer timer("WH_Correlator::process()", true);

  int			  remap[NR_STATIONS];
  DH_CorrCube::BufferType *input[NR_STATIONS];
  DH_Vis::BufferType	  *output = static_cast<DH_Vis*>(getDataManager().getOutHolder(0))->getBuffer();

  for (int stat = 0; stat < NR_STATIONS; stat ++) {
    input[stat] = static_cast<DH_CorrCube*>(getDataManager().getInHolder(stat / MAX_STATIONS_PER_PPF))->getBuffer();
    remap[stat] = stat % MAX_STATIONS_PER_PPF;
  }

  timer.start();
#if defined C_IMPLEMENTATION
  // C++ reference implementation

  for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch ++) {
    for (int stat2 = 0; stat2 < NR_STATIONS; stat2 ++) {
      for (int stat1 = 0; stat1 <= stat2; stat1 ++) { 
	for (int pol1 = 0; pol1 < 2; pol1 ++) {
	  for (int pol2 = 0; pol2 < 2; pol2 ++) {
	    dcomplex sum = makedcomplex(0, 0);

	    for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time ++) {
	      sum += (*input[stat1])[ch][remap[stat1]][time][pol1] * ~(*input[stat2])[ch][remap[stat2]][time][pol2];
	    }

	    (*output)[DH_Vis::baseline(stat1,stat2)][ch][pol1][pol2] = sum;
	  }
	}
      }
    }
  }
#else
  // Blue Gene/L assembler version. 
  // Divide the correlation matrix into blocks of 2x2. 
  // Correlate the entire block (all time samples) before 
  // going on to the next block.

  for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch ++) {
    for (int stat2 = NR_STATIONS % 2 ? 1 : 2; stat2 < NR_STATIONS; stat2 += 2) {
      int stat1 = 0;
      // do as many 3x2 blocks as possible
      for (; stat1 < stat2 - 4 || (stat1 & 1) != 0; stat1 += 3) {
	_correlate_3x2(&(*input[stat1  ])[ch][remap[stat1  ]],
		       &(*input[stat1+1])[ch][remap[stat1+1]],
		       &(*input[stat1+2])[ch][remap[stat1+2]],
		       &(*input[stat2  ])[ch][remap[stat2  ]],
		       &(*input[stat2+1])[ch][remap[stat2+1]],
		       &(*output)[DH_Vis::baseline(stat1  , stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1  , stat2+1)][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2+1)][ch],
		       &(*output)[DH_Vis::baseline(stat1+2, stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1+2, stat2+1)][ch]);
      }
      // see if some 2x2 blocks are necessary
      for (; stat1 < stat2; stat1 += 2) {
	_correlate_2x2(&(*input[stat1  ])[ch][remap[stat1  ]],
		       &(*input[stat1+1])[ch][remap[stat1+1]],
		       &(*input[stat2  ])[ch][remap[stat2  ]],
		       &(*input[stat2+1])[ch][remap[stat2+1]],
		       &(*output)[DH_Vis::baseline(stat1  , stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1  , stat2+1)][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2+1)][ch]);
      }
    }

    // do the remaining autocorrelations
    for (int stat = 0; stat < NR_STATIONS; stat += 2) {
#if NR_STATIONS % 2 == 0
      _correlate_1_and_2(&(*input[stat  ])[ch][remap[stat]],
			 &(*input[stat+1])[ch][remap[stat+1]],
			 &(*output)[DH_Vis::baseline(stat  , stat  )][ch],
			 &(*output)[DH_Vis::baseline(stat  , stat+1)][ch],
			 &(*output)[DH_Vis::baseline(stat+1, stat+1)][ch]);
#else
      _auto_correlate_1x1(&(*input[stat])[ch][remap[stat]],
			  &(*output)[DH_Vis::baseline(stat,stat)][ch]);
#endif
    }
  }
#endif  
  timer.stop();
}

void WH_Correlator::postprocess()
{
}

void WH_Correlator::dump() const
{
}
