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
#include <WH_Correlator.h>
#include <Correlator.h>

#if !defined HAVE_BGL
#define C_IMPLEMENTATION
#endif

using namespace LOFAR;


extern "C"
{
  void _fast_memcpy(void *dst, const void *src, size_t size);
}

WH_Correlator::WH_Correlator(const string &name)
:
  WorkHolder(NR_PPF_PER_COMPUTE_CELL, 1, name, "WH_Correlator"),
  itsInputBuffer(0)
{
  ACC::APS::ParameterSet myPS("TFlopCorrelator.cfg");

  itsNfilters = myPS.getInt32("PPF.NrFilters");
  itsNsamples = myPS.getInt32("PPF.NrStationSamples");
  itsNelements = myPS.getInt32("PPF.NrStations");
  itsNpolarisations = myPS.getInt32("PPF.NrPolarizations");
  itsNchannels = myPS.getInt32("PPF.NrSubChannels") / myPS.getInt32("PPF.NrCorrelatorsPerFilter");
    

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
    getDataManager().addInDataHolder(i, new DH_CorrCube(str, 0));
    //totalInputSize += static_cast<DH_CorrCube*>(getDataManager().getInHolder(i))->getBufSize();
  }

//   for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch ++) {
//     char str[50];
//     snprintf(str, 50, "output_%d_of_%d", ch, NR_CHANNELS_PER_CORRELATOR);
//     getDataManager().addOutDataHolder(ch, new DH_Vis(str, 0, myPS));
//   }
  getDataManager().addOutDataHolder(0, new DH_Vis("output", 0, myPS));

}

WH_Correlator::~WH_Correlator()
{
#if NR_PPF_PER_COMPUTE_CELL == 2
  delete itsInputBuffer;
#endif
}

WorkHolder* WH_Correlator::construct(const string& name)
{
  return new WH_Correlator(name);
}

WH_Correlator* WH_Correlator::make(const string& name)
{
  return new WH_Correlator(name);
}

void WH_Correlator::preprocess()
{ 
#if NR_PPF_PER_COMPUTE_CELL == 2
  itsInputBuffer = new DH_CorrCube::BufferType[1];
#endif
}

void WH_Correlator::process()
{
  static NSTimer timer("WH_Correlator::process()", true);

  DH_CorrCube::BufferType *input  = static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBuffer();
  DH_Vis::BufferType	  *output = static_cast<DH_Vis*>(getDataManager().getOutHolder(0))->getBuffer();
  int bufSize = static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBufSize();

  timer.start();
#if 0
  // C++ reference implementation

#if NR_PPF_PER_COMPUTE_CELL == 1
  itsInputBuffer = static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBuffer();
#elif NR_PPF_PER_COMPUTE_CELL == 2
  /// Unfortunately we need to reassamble the input matrix, which requires a 20Mb memcpy
  /// (could this be done more efficiently?)
  /// Currently we hardcore 2 inputs.
  memcpy(itsInputBuffer, static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBuffer(), bufSize*sizeof(fcomplex));
  memcpy(itsInputBuffer+1, static_cast<DH_CorrCube*>(getDataManager().getInHolder(1))->getBuffer(), (17*bufSize/18) * sizeof(fcomplex));
#endif

  for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch ++) {
    for (int stat2 = 0; stat2 < NR_STATIONS; stat2 ++) {
      for (int stat1 = 0; stat1 <= stat2; stat1 ++) { 
	for (int pol1 = 0; pol1 < 2; pol1 ++) {
	  for (int pol2 = 0; pol2 < 2; pol2 ++) {
	    fcomplex sum = makefcomplex(0, 0);

	    for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time ++) {
	      sum += (*itsInputBuffer)[ch][stat1][time][pol1] * ~(*itsInputBuffer)[ch][stat2][time][pol2];
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

#if NR_PPF_PER_COMPUTE_CELL == 1
  itsInputBuffer = static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBuffer();
#elif NR_PPF_PER_COMPUTE_CELL == 2
  /// Unfortunately we need to reassamble the input matrix, which requires a 20Mb memcpy
  /// (could this be done more efficiently?)
  /// Currently we hardcore 2 inputs.
  _fast_memcpy(itsInputBuffer, static_cast<DH_CorrCube*>(getDataManager().getInHolder(0))->getBuffer(), bufSize*sizeof(fcomplex));
  _fast_memcpy(itsInputBuffer+1, static_cast<DH_CorrCube*>(getDataManager().getInHolder(1))->getBuffer(), (17*bufSize/18) * sizeof(fcomplex));
#endif

  for (int ch = 0; ch < NR_CHANNELS_PER_CORRELATOR; ch ++) {
    for (int stat2 = NR_STATIONS % 2 ? 1 : 2; stat2 < NR_STATIONS; stat2 += 2) {
      int stat1 = 0;
      // do as many 3x2 blocks as possible
      for (; stat1 < stat2 - 4 || (stat1 & 1) != 0; stat1 += 3) {
	_correlate_3x2(&(*itsInputBuffer)[ch][stat2  ],
		       &(*itsInputBuffer)[ch][stat2+1],
		       &(*itsInputBuffer)[ch][stat1  ],
		       &(*itsInputBuffer)[ch][stat1+1],
		       &(*itsInputBuffer)[ch][stat1+2],
		       &(*output)[DH_Vis::baseline(stat1  , stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1  , stat2+1)][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2+1)][ch],
		       &(*output)[DH_Vis::baseline(stat1+2, stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1+2, stat2+1)][ch]);
      }
      // see if some 2x2 blocks are necessary
      for (; stat1 < stat2; stat1 += 2) {
	_correlate_2x2(&(*itsInputBuffer)[ch][stat2  ],
		       &(*itsInputBuffer)[ch][stat2+1],
		       &(*itsInputBuffer)[ch][stat1  ],
		       &(*itsInputBuffer)[ch][stat1+1],
		       &(*output)[DH_Vis::baseline(stat1  , stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1  , stat2+1)][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2  )][ch],
		       &(*output)[DH_Vis::baseline(stat1+1, stat2+1)][ch]);
      }
    }

    // do the remaining autocorrelations
    for (int stat = 0; stat < NR_STATIONS; stat += 2) {
#if NR_STATIONS % 2 == 0
      _auto_correlate_1_and_2(&(*itsInputBuffer)[ch][stat],
			      &(*itsInputBuffer)[ch][stat+1],
			      &(*output)[DH_Vis::baseline(stat  , stat  )][ch],
			      &(*output)[DH_Vis::baseline(stat  , stat+1)][ch],
			      &(*output)[DH_Vis::baseline(stat+1, stat+1)][ch]);
#else
      _auto_correlate_1x1(&(*itsInputBuffer)[ch][stat],
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
