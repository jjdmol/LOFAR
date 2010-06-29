//# ObsDomain.cc: Define the shape of a domain
//#
//# Copyright (c) 2007
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
//# $Id$

#include <lofar_config.h>

#include <MWCommon/ObsDomain.h>

namespace LOFAR { namespace CEP {

  ObsDomain::ObsDomain()
    : itsStartFreq (-1),
      itsEndFreq   (1e30),
      itsStartTime (-1),
      itsEndTime   (1e30)
  {}

  ObsDomain::ObsDomain (const ObsDomain& fullDomain,
			const DomainShape& workDomainShape)
  {
    // Construct the first work domain from the full observation domain
    // and the work domain shape.
    double freqLen = workDomainShape.getFreqSize();
    double timeLen = workDomainShape.getTimeSize();
    itsStartFreq = fullDomain.getStartFreq();
    itsEndFreq   = std::min(fullDomain.getEndFreq(),
			    itsStartFreq+freqLen);
    itsStartTime = fullDomain.getStartTime();
    itsEndTime   = std::min(fullDomain.getEndTime(),
			    itsStartTime+timeLen);
  }

  void ObsDomain::setFreq (double startFreq, double endFreq)
  {
    itsStartFreq = startFreq;
    itsEndFreq   = endFreq;
  }
  
  void ObsDomain::setTime (double startTime, double endTime)
  {
    itsStartTime = startTime;
    itsEndTime   = endTime;
  }

  bool ObsDomain::getNextWorkDomain (ObsDomain& workDomain,
				     const DomainShape& workDomainShape) const
  {
    double freqLen = workDomainShape.getFreqSize();
    double timeLen = workDomainShape.getTimeSize();
    // First time?
    if (workDomain.getStartFreq() < 0) {
      workDomain.setFreq (itsStartFreq,
			  std::min(itsEndFreq, itsStartFreq+freqLen));
      workDomain.setTime (itsStartTime,
			  std::min(itsEndTime, itsStartTime+timeLen));
      return true;
    }
    // Increment in frequency if possible.
    double sfreq = workDomain.getStartFreq() + freqLen;
    if (sfreq < itsEndFreq) {
      workDomain.setFreq (sfreq, std::min(itsEndFreq, sfreq+freqLen));
      return true;
    }
    double stime = workDomain.getStartTime() + timeLen;
    if (stime < itsEndTime) {
      // Reset work domain (for freq) and set times.
      workDomain = ObsDomain(*this, workDomainShape);
      workDomain.setTime (stime, std::min(itsEndTime, stime+timeLen));
      return true;
    }
    return false;
  }

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const ObsDomain& domain)
  {
    bs << domain.itsStartFreq << domain.itsEndFreq
       << domain.itsStartTime << domain.itsEndTime;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  ObsDomain& domain)
  {
    bs >> domain.itsStartFreq >> domain.itsEndFreq
       >> domain.itsStartTime >> domain.itsEndTime;
    return bs;
  }

  std::ostream& operator<< (std::ostream& os,
			    const ObsDomain& domain)
  {
    os << '[' << domain.itsStartFreq << " Hz, " << domain.itsEndFreq
       << " Hz, " << domain.itsStartTime << ", " << domain.itsEndTime << ']';
    return os;
  }


}} // end namespaces
