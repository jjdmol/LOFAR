//# MSWriterCorrelated.cc: a writer for correlated visibilities
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "MSWriterCorrelated.h"

#include <sys/types.h>
#include <fcntl.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <MSLofar/FailedTileInfo.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/LTAFeedback.h>

#include <tables/Tables/Table.h>
#include <casa/Quanta/MVTime.h>

#include "MeasurementSetFormat.h"

using boost::format;
using namespace casa;


namespace LOFAR
{
  namespace Cobalt
  {

    MSWriterCorrelated::MSWriterCorrelated (const std::string &logPrefix, const std::string &msName, const Parset &parset, unsigned subbandIndex)
      :
      MSWriterFile(str(format("%s/table.f0data") % msName)),
      itsLogPrefix(logPrefix),
      itsMSname(msName),
      itsParset(parset),
      itsSubbandIndex(subbandIndex)
    {
      // Add file-specific processing feedback
      LTAFeedback fb(itsParset.settings);
      itsConfiguration.adoptCollection(fb.correlatedFeedback(itsSubbandIndex));
      itsConfigurationPrefix = fb.correlatedPrefix(itsSubbandIndex);

      // Create Sequence file
      if (LofarStManVersion > 1) {
        string seqfilename = str(format("%s/table.f0seqnr") % msName);

        try {
          itsSequenceNumbersFile = new FileStream(seqfilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        } catch (...) {
          LOG_WARN_STR(itsLogPrefix << "Could not open sequence numbers file " << seqfilename);
        }
      }
    }


    void MSWriterCorrelated::createMetaData()
    {
      // Creaate MeasurementSet
#if defined HAVE_AIPSPP
      MeasurementSetFormat myFormat(itsParset, 512);

      myFormat.addSubband(itsMSname, itsSubbandIndex);

      LOG_DEBUG_STR(itsLogPrefix << "MeasurementSet created");
#endif // defined HAVE_AIPSPP
    }


    MSWriterCorrelated::~MSWriterCorrelated()
    {
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
        // quick fix: always write to maintain integrity
        unsigned seqnr = data->sequenceNumber(true);

        itsSequenceNumbersFile->write(&seqnr, sizeof seqnr);
      }

      itsNrBlocksWritten++;

      itsConfiguration.replace(itsConfigurationPrefix + "size",     str(format("%u") % getDataSize()));
      itsConfiguration.replace(itsConfigurationPrefix + "duration", str(format("%f") % ((data->sequenceNumber() + 1) * itsParset.settings.correlator.integrationTime())));
      itsConfiguration.replace(itsConfigurationPrefix + "percentageWritten", str(format("%u") % percentageWritten()));
    }


    static MVEpoch datetime2epoch(const string &datetime)
    {
      Quantity q;

      if (!MVTime::read(q, datetime))
        return MVEpoch(0);

      return MVEpoch(q);
    }


    void MSWriterCorrelated::augment(const FinalMetaData &finalMetaData)
    {
      ScopedLock sl(MeasurementSetFormat::sharedMutex);

      map<string, FailedTileInfo::VectorFailed> brokenBefore, brokenDuring;

      // fill set of broken hardware at beginning of observation
      for (size_t i = 0; i < finalMetaData.brokenRCUsAtBegin.size(); i++) {
        const struct FinalMetaData::BrokenRCU &rcu = finalMetaData.brokenRCUsAtBegin[i];

        brokenBefore[rcu.station].push_back(FailedTileInfo(rcu.station, rcu.time, datetime2epoch(rcu.time), rcu.type, rcu.seqnr));
      }

      // fill set of hardware that broke during the observation
      for (size_t i = 0; i < finalMetaData.brokenRCUsDuring.size(); i++) {
        const struct FinalMetaData::BrokenRCU &rcu = finalMetaData.brokenRCUsDuring[i];

        brokenDuring[rcu.station].push_back(FailedTileInfo(rcu.station, rcu.time, datetime2epoch(rcu.time), rcu.type, rcu.seqnr));
      }

      LOG_DEBUG_STR(itsLogPrefix << "Reopening MeasurementSet");

      Table ms(itsMSname, Table::Update);

      vector<FailedTileInfo::VectorFailed> before(FailedTileInfo::antennaConvert(ms, brokenBefore));
      vector<FailedTileInfo::VectorFailed> during(FailedTileInfo::antennaConvert(ms, brokenDuring));

      LOG_DEBUG_STR(itsLogPrefix << "Writing broken hardware information to MeasurementSet");

      try {
        FailedTileInfo::writeFailed(ms, before, during);
      } catch (Exception &ex) {
        LOG_ERROR_STR("Failed to write broken hardware information: " << ex);
      }
    }


  } // namespace Cobalt
} // namespace LOFAR

