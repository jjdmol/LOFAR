//# DomainShape.cc: Define the shape of a domain
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

#include <MWCommon/DomainShape.h>

namespace LOFAR { namespace CEP {

  DomainShape::DomainShape()
    : itsFreqSize (1e30),
      itsTimeSize (1e30)
  {}

  DomainShape::DomainShape (double freqSize, double timeSize)
    : itsFreqSize (freqSize),
      itsTimeSize (timeSize)
  {}

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const DomainShape& ds)
  {
    bs << ds.itsFreqSize << ds.itsTimeSize;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  DomainShape& ds)
  {
    bs >> ds.itsFreqSize >> ds.itsTimeSize;
    return bs;
  }

  std::ostream& operator<< (std::ostream& os,
			    const DomainShape& ds)
  {
    os << ds.itsFreqSize << " Hz,  " << ds.itsTimeSize << " s";
    return os;
  }

}} // end namespaces
