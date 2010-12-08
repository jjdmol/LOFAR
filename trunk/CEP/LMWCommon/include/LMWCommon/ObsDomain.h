//# ObsDomain.h:  Define the boundary values of a domain
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_LMWCOMMON_OBSDOMAIN_H
#define LOFAR_LMWCOMMON_OBSDOMAIN_H

// @file
// @brief Define the boundary values of a domain.
// @author Ger van Diepen (diepen AT astron nl)

#include <LMWCommon/DomainShape.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <iosfwd>

namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief Define the boundary values of a domain.

  // This class defines the boundaries of an observation domain.
  // Currently it only defines a single range in time and freq.
  // In the future it will probably need to be extended to multiple bands.
  //
  // Furthermore it offers a function to get the next work domain
  // given a work domain shape defined by a DomainShape object.
  //  The master control uses this function to iterate over work domains.

  class ObsDomain
  {
  public:
    // Set default shape to all frequencies and times.
    ObsDomain();

    // Form the starting work domain from the full observation domain
    // and the work domain shape.
    ObsDomain (const ObsDomain& fullDomain,
	       const DomainShape& workDomainShape);

    // Set frequency range (in Hz).
    void setFreq (double startFreq, double endFreq);

    // Set time range (in sec).
    void setTime (double startTime, double endTime);

    // Get the values.
    // @{
    double getStartFreq() const
      { return itsStartFreq; }
    double getEndFreq() const
      { return itsEndFreq; }
    double getStartTime() const
      { return itsStartTime; }
    double getEndTime() const
      { return itsEndTime; }
    // @}

    // Go to the next work domain.
    // Return false if no more work domains.
    bool getNextWorkDomain (ObsDomain& workDomain,
			    const DomainShape& workDomainShape) const;

    // Convert to/from blob.
    // @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
					   const ObsDomain& ds);
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
					   ObsDomain&);
    // @}

    // Print the object.
    friend std::ostream& operator<< (std::ostream& os,
				     const ObsDomain& ds);

  private:
    double itsStartFreq;
    double itsEndFreq;
    double itsStartTime;
    double itsEndTime;
  };

}} //# end namespaces

#endif
