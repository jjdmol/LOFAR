//# MeqWsrtPoint.h: The total baseline prediction of point sources in the WSRT model
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

#if !defined(MNS_MEQWSRTPOINT_H)
#define MNS_MEQWSRTPOINT_H

//# Includes
#include <MNS/MeqPointSource.h>
#include <MNS/MeqResult.h>
#include <Common/lofar_vector.h>

//# Forward Declarations
class MeqPointDFT;


class MeqWsrtPoint
{
public:
  // The expressions give the coefficients of the 2-dim polynomial.
  // nx and ny give the number of coefficients in x and y.
  MeqWsrtPoint (const vector<MeqPointSource>& sources, MeqPointDFT* dft);
       
  ~MeqWsrtPoint();

  // Calculate the results for the given domain.
  void calcResult (const MeqRequest&);

  // Get the number of cells needed.
  const vector<int>& ncells() const
    { return itsNcell; }

  // Get the results of the expression.
  const MeqResult& getResultXX() const
    { return itsXX; }
  const MeqResult& getResultXY() const
    { return itsXY; }
  const MeqResult& getResultYX() const
    { return itsYX; }
  const MeqResult& getResultYY() const
    { return itsYY; }

private:
  vector<MeqPointSource> itsSources;
  MeqPointDFT*           itsDFT;
  vector<int>            itsNcell;
  MeqResult              itsXX;
  MeqResult              itsXY;
  MeqResult              itsYX;
  MeqResult              itsYY;
};


#endif
