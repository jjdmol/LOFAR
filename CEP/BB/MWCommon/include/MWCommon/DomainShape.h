//# DomainShape.h: Define the shape of a domain
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_MWCOMMON_DOMAINSHAPE_H
#define LOFAR_MWCOMMON_DOMAINSHAPE_H

// @file
// @brief Define the shape of a domain.
// @author Ger van Diepen (diepen AT astron nl)

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <iosfwd>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Define the shape of a domain.

  // This class defines the shape of a domain.
  // Currently this can only be done for time and frequency.
  //
  // This object can be used by ObsDomain to iterate over its observation
  // domain in chunk of this domain shape.

  class DomainShape
  {
  public:
    // Set default shape to all frequencies and times.
    DomainShape();

    // Set from frequency in Hz and time in sec.
    DomainShape (double freqSize, double timeSize);

    // Get the shape.
    // @{
    double getFreqSize() const 
      { return itsFreqSize; }
    double getTimeSize() const 
      { return itsTimeSize; }
    // @}

    // Convert to/from blob.
    // @{
    friend LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream&,
					   const DomainShape&);
    friend LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream&,
					   DomainShape&);
    // @}

    // Print.
    friend std::ostream& operator<< (std::ostream&,
				     const DomainShape&);

  private:
    double itsFreqSize;
    double itsTimeSize;
  };

}} //# end namespaces

#endif
