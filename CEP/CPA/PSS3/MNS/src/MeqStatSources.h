//# MeqStatSources.h: The precalculated source DFT exponents for a domain
//#
//# Copyright (C) 2002
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

#if !defined(MNS_MEQSTATSOURCES_H)
#define MNS_MEQSTATSOURCES_H

//# Includes
#include <MNS/MeqResult.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqPointSource.h>
#include <Common/lofar_vector.h>

//# Forward declarations
class MeqStatUVW;
class MeqPhaseRef;


class MeqStatSources
{
public:
  // The default constructor.
  MeqStatSources()
    {};

  MeqStatSources (MeqStatUVW*,
		  vector<MeqPointSource>*);

  void calculate (const MeqRequest&);

  // Get the result for the given source.
  const MeqResult& getResult (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request);
      return itsResults[request.getSourceNr()]; }

  // Get N (= sqrt(1-l^2-m^2) for the given source.
  const MeqResult& getN (const MeqRequest& request)
    { return (*itsSources)[request.getSourceNr()].getN(request); }

  // Get the exponent for the given time value in the domain.
  double getExponent (int sourceNr, const MeqRequest& request);

  // Get access to the UVW.
  MeqStatUVW& uvw()
    { return *itsUVW; }

private:
  MeqStatUVW*  itsUVW;
  vector<MeqPointSource>* itsSources;
  vector<MeqResult> itsResults;
  MeqRequestId itsLastReqId;
};


#endif
