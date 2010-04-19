//# WorkDomainSpec.cc: Define the specifications of the work domain
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

#include <MWCommon/WorkDomainSpec.h>
#include <MWCommon/MWError.h>
#include <Blob/BlobArray.h>

using namespace std;


namespace LOFAR { namespace CEP {

  void WorkDomainSpec::setAntennas (const vector<int>& antNrs)
  {
    itsAntNrs = antNrs;
  }

  void WorkDomainSpec::setAntennaNames (const vector<string>& antNames)
  {
    itsAntNames = antNames;
  }

  void WorkDomainSpec::setCorr (const vector<bool>& corr)
  {
    itsCorr = corr;
  }

  LOFAR::BlobOStream& operator<< (LOFAR::BlobOStream& bs,
				  const WorkDomainSpec& wds)
  {
    bs.putStart ("WDS", 1);
    bs << wds.itsInColumn
       << wds.itsAntNrs
       << wds.itsAntNames
       << wds.itsAutoCorr
       << wds.itsCorr
       << wds.itsShape
       << wds.itsFreqInt
       << wds.itsTimeInt;
    return bs;
  }

  LOFAR::BlobIStream& operator>> (LOFAR::BlobIStream& bs,
				  WorkDomainSpec& wds)
  {
    int vers = bs.getStart ("WDS");
    ASSERT (vers == 1);
    bs >> wds.itsInColumn
       >> wds.itsAntNrs
       >> wds.itsAntNames
       >> wds.itsAutoCorr
       >> wds.itsCorr
       >> wds.itsShape
       >> wds.itsFreqInt
       >> wds.itsTimeInt;
    return bs;
  }


}} // end namespaces
