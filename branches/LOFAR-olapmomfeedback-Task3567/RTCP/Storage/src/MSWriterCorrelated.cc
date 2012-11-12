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
#include <Common/SystemUtil.h>
#include <Storage/MSWriterCorrelated.h>
#include <Interface/CorrelatedData.h>
#include <vector>
#include <string>
#include <boost/format.hpp>

using boost::format;

namespace LOFAR {
namespace RTCP {

MSWriterCorrelated::MSWriterCorrelated (const string &msName, const Parset &parset, unsigned streamNr)
:
 MSWriterFile(msName),
 itsParset(parset)
{
  itsNrExpectedBlocks = itsParset.nrCorrelatedBlocks();

  std::vector<std::string> stationNames = parset.mergedStationNames();
  std::vector<std::string> baselineNames(parset.nrBaselines());
  unsigned nrStations = stationNames.size();

  // order of baselines as station indices:
  // 0-0, 1-0, 1-1, 2-0, 2-1, 2-2 ... (see RTCP/CNProc/Correlator.cc)

  unsigned bl = 0;

  for(unsigned s1 = 0; s1 < nrStations; s1++)
    for(unsigned s2 = 0; s2 <= s1; s2++)
      baselineNames[bl++] = str(format("%s_%s") % stationNames[s1] % stationNames[s2]);

  const vector<unsigned> subbands  = itsParset.subbandList();
  const vector<unsigned> SAPs      = itsParset.subbandToSAPmapping();
  const vector<double> frequencies = itsParset.subbandToFrequencyMapping();

  itsConfiguration.add("fileFormat",           "AIPS++/CASA");
  itsConfiguration.add("filename",             LOFAR::basename(msName));
  itsConfiguration.add("size",                 "0");
  itsConfiguration.add("location",             parset.getHostName(CORRELATED_DATA, streamNr) + ":" + LOFAR::dirname(msName));

  itsConfiguration.add("percentageWritten",    "0");
  itsConfiguration.add("startTime",            parset.getString("Observation.startTime"));
  itsConfiguration.add("duration",             "0");
  itsConfiguration.add("integrationInterval",  str(format("%lf") % parset.IONintegrationTime()));
  itsConfiguration.add("centralFrequency",     str(format("%lf") % (frequencies[streamNr]/1e6)));
  itsConfiguration.add("channelWidth",         str(format("%lf") % (parset.channelWidth()/1e3)));
  itsConfiguration.add("channelsPerSubband",   str(format("%u") % parset.nrChannelsPerSubband()));
  itsConfiguration.add("stationSubband",       str(format("%u") % subbands[streamNr]));
  itsConfiguration.add("subband",              str(format("%u") % streamNr));
  itsConfiguration.add("SAP",                  str(format("%u") % SAPs[streamNr]));
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

  itsNrBlocksWritten++;

  itsConfiguration.replace("size",     str(format("%ll") % getDataSize()));
  itsConfiguration.replace("duration", str(format("%lf") % ((data->sequenceNumber() + 1) * itsParset.IONintegrationTime())));
  itsConfiguration.replace("percentageWritten", str(format("%u") % percentageWritten()));
}


} // namespace RTCP
} // namespace LOFAR

