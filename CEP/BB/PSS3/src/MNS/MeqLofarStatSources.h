//# MeqLofarStatSources.h: The Jones expressions for all sources of a station
//#
//# Copyright (C) 2003
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

#if !defined(MNS_MEQLOFARSTATSOURCES_H)
#define MNS_MEQLOFARSTATSOURCES_H

//# Includes
#include <PSS3/MNS/MeqJonesExpr.h>
#include <PSS3/MNS/MeqStatSources.h>
#include <Common/lofar_vector.h>

namespace LOFAR {

// This class represents the Jones matrix for all sources for a station.

class MeqLofarStatSources
{
public:
  // Construct from the station Jones matrices per source and the
  // list of sources.
  MeqLofarStatSources (const vector<MeqJonesExpr*>& statSources,
		       MeqStatSources* sources);

  virtual ~MeqLofarStatSources();

  // Get the result for the given source.
  // First calculate the results if a new request id is given.
  const MeqResult& getResult11 (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) {
        calcResult(request);
      }
      int srcnr = itsSrc->actualSourceNr (request.getSourceNr());
      return itsStat[srcnr]->getResult11(); }
  const MeqResult& getResult12 (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) {
        calcResult(request);
      }
      int srcnr = itsSrc->actualSourceNr (request.getSourceNr());
      return itsStat[srcnr]->getResult12(); }
  const MeqResult& getResult21 (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) {
        calcResult(request);
      }
      int srcnr = itsSrc->actualSourceNr (request.getSourceNr());
      return itsStat[srcnr]->getResult21(); }
  const MeqResult& getResult22 (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) {
        calcResult(request);
      }
      int srcnr = itsSrc->actualSourceNr (request.getSourceNr());
      return itsStat[srcnr]->getResult22(); }

  const MeqResult& getN (const MeqRequest& request)
    { return itsSrc->getN(request); }

private:
  // Calculate the result of its members.
  void calcResult (const MeqRequest&);

  vector<MeqJonesExpr*> itsStat;
  MeqStatSources*       itsSrc;
  MeqMatrix             itsK;
  MeqMatrix             itsPertK;
  MeqRequestId          itsLastReqId;
};

}

#endif
