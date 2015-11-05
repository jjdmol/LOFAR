//# MSPollingReader.cc: DPPP step reading from an MS being filled
//# Copyright (C) 2010
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id: MSReader.cc 27640 2013-12-04 08:02:49Z diepen $
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/MSPollingReader.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <unistd.h>    //# for usleep

using namespace casa;

namespace LOFAR {
  namespace DPPP {

    MSPollingReader::MSPollingReader()
    {}

    MSPollingReader::MSPollingReader (const string& msName,
                                      const ParameterSet& parset,
                                      const string& prefix)
      : MSReader (msName, parset, prefix, false, false),
        itsPollingTime  (parset.getDouble(prefix+"pollingtime", 0.1)),
        itsMaxWaitTime  (parset.getDouble(prefix+"maxwaittime", 10.)),
        itsEndOfData    (false)
    {
      // Check that no selection on spw or baseline is given.
      ASSERTSTR (itsSelBL.empty()  &&  itsSpw < 0,
                 "PollingMSReader: baseline or spw selection cannot be done");
      // Sorting cannot be done (because MS will grow).
      itsNeedSort = false;
      // Open the MS table without locking.
      // DPPP could be started before the MS exists, so poll for some time.
      // We have to wait until it contains some data.
      int nwait = Int(itsMaxWaitTime/itsPollingTime + 0.99);
      int nr = 0;
      while (! Table::isReadable (msName)  ||
             Table(msName, TableLock::AutoNoReadLocking).nrow() == 0) {
        nr++;
        ASSERTSTR (nr<=nwait, msName << " does not exist");
        usleep (itsPollingTime*1e6);
      }
      ///      // Sleep a bit more to ensure the data files are flushed.
      ///      usleep (itsPollingTime);
      // Now let the base class MSReader do the initialization.
      openMS (msName);
      // If not given, get the last time from the observation table.
      ///if (itsEndTimeStr.empty()) {
      ///itsLastTime = getLastTimeFromObs();
      ///}
    }

    double MSPollingReader::getLastTimeFromObs() const
    {
      Table obsTab = Table(itsMSName + "/OBSERVATION");
      ASSERT (obsTab.nrow() > 0);
      // Get time range from first row.
      ROArrayColumn<Double> timeRange(obsTab, "TIME_RANGE");
      Vector<double> times = timeRange(0);
      ASSERT (times.size() == 2);
      // Return middle of last interval.
      return times[1] - 0.5 * itsTimeInterval;
    }

    MSPollingReader::~MSPollingReader()
    {}

    void MSPollingReader::show (std::ostream& os) const
    {
      os << "MSPollingReader" << std::endl;
      showParm (os, uint((itsLastTime-itsFirstTime)/itsTimeInterval));
      os << "  pollingtime     " << itsPollingTime << std::endl;
      os << "  maxwaittime     " << itsMaxWaitTime << std::endl;
    }

    bool MSPollingReader::canUpdateMS() const
    {
      return false;
    }

    bool MSPollingReader::doGetNext()
    {
      ASSERT (!itsEndOfData);
      // Resync the MS if at the end of the iterator.
      itsIter.next();
      if (itsIter.pastEnd()) {
        // No more data available, so try to resync until timeout.
        Int64 nneeded = (itsNrRead+1) * getInfo().nbaselines();
        itsMS.resync();
        int nr = 0;
        int nwait = int(itsMaxWaitTime/itsPollingTime + 0.99);
        while (itsMS.nrow() < nneeded) {
          // Try a resync until timeout.
          if (++nr > nwait) {
            // timeout; no more data
            itsEndOfData = true;
            return false;
          }
          usleep (itsPollingTime*1e6);
          itsMS.resync();
        }
        // More data available, so set iterator to next time slot.
        // Also set the new last time if not explicitly given.
        itsIter.next();
        if (itsEndTimeStr.empty()) {
          itsLastTime = ROScalarColumn<double>(itsMS, "TIME")(itsMS.nrow()-1);
        }
      }
      return true;
    }

  } //# end namespace
}
