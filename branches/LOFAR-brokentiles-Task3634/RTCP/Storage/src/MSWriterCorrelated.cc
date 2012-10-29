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
#include <Interface/CorrelatedData.h>
#include <Beaminfo/FailedTileInfo.h>
#include <Storage/MSWriterCorrelated.h>
#include <Storage/MeasurementSetFormat.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <sys/types.h>

#include <boost/format.hpp>
using boost::format;

namespace LOFAR {
namespace RTCP {

MSWriterCorrelated::MSWriterCorrelated (const std::string &logPrefix, const std::string &msName, const Parset &parset, unsigned subbandIndex, bool isBigEndian)
:
  MSWriterFile(str(format("%s/table.f0data") % msName)),
  itsLogPrefix(logPrefix),
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

  // Make MeasurementSet filestructures and required tables

#if defined HAVE_AIPSPP
  MeasurementSetFormat myFormat(itsParset, 512);

  myFormat.addSubband(msName, subbandIndex, isBigEndian);

  LOG_INFO_STR(itsLogPrefix << "MeasurementSet created");
#endif // defined HAVE_AIPSPP

  if (itsParset.getLofarStManVersion() > 1) {
    string seqfilename = str(format("%s/table.f0seqnr") % msName);
    
    try {
      itsSequenceNumbersFile = new FileStream(seqfilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
    } catch (...) {
      LOG_WARN_STR(itsLogPrefix << "Could not open sequence numbers file " << seqfilename);
    }
  }
}


MSWriterCorrelated::~MSWriterCorrelated()
{
  flushSequenceNumbers();
}


void MSWriterCorrelated::write(StreamableData *data)
{
  CorrelatedData *cdata = dynamic_cast<CorrelatedData*>(data);

  ASSERT( data );
  ASSERT( cdata );

  // Write data
  MSWriterFile::write(data);

  // Write sequence number
  if (itsSequenceNumbersFile != 0) {
    // write the sequencenumber in correlator endianness, no byteswapping
    itsSequenceNumbers.push_back(data->sequenceNumber(true));
    
    if (itsSequenceNumbers.size() > 64)
      flushSequenceNumbers();
  }
}


void MSWriterCorrelated::flushSequenceNumbers()
{
  if (itsSequenceNumbersFile != 0) {
    LOG_INFO_STR(itsLogPrefix << "Flushing sequence numbers");
    itsSequenceNumbersFile->write(itsSequenceNumbers.data(), itsSequenceNumbers.size() * sizeof(unsigned));
    itsSequenceNumbers.clear();
  }
}


void MSWriterCorrelated::augment(const FinalMetaData &finalMetaData)
{
  // TODO
}


} // namespace RTCP
} // namespace LOFAR

