//# MSWriterCorrelated: a writer for correlated visibilities
//#
//#  Copyright (C) 2001
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
//#  $Id: $

#include <lofar_config.h>
#include <Storage/MSWriterCorrelated.h>
#include <Interface/CorrelatedData.h>
#include <Beaminfo/FailedTileInfo.h>
#include <vector>
#include <string>
#include <boost/format.hpp>

using boost::format;

namespace LOFAR {
namespace RTCP {

MSWriterCorrelated::MSWriterCorrelated (const string &msName, const Parset &parset)
:
 MSWriterFile(msName),
 itsParset(parset)
{
  std::vector<std::string> stationNames = parset.mergedStationNames();
  std::vector<std::string> baselineNames(parset.nrBaselines());
  unsigned nrStations = stationNames.size();

  // order of baselines as station indices:
  // 0-0, 1-0, 1-1, 2-0, 2-1, 2-2 ... (see RTCP/CNProc/Correlator.cc)

  unsigned bl = 0;

  for(unsigned s1 = 0; s1 < nrStations; s1++)
    for(unsigned s2 = 0; s2 <= s1; s2++)
      baselineNames[bl++] = str(format("%s_%s") % stationNames[s1] % stationNames[s2]);
}


MSWriterCorrelated::~MSWriterCorrelated()
{
}


void MSWriterCorrelated::write(StreamableData *data)
{
  CorrelatedData *cdata = dynamic_cast<CorrelatedData*>(data);

  ASSERT( data );
  ASSERT( cdata );

  MSWriterFile::write(data);
}


void MSWriterCorrelated::augment(const FinalMetaData &finalMetaData)
{
  // TODO
}


} // namespace RTCP
} // namespace LOFAR

