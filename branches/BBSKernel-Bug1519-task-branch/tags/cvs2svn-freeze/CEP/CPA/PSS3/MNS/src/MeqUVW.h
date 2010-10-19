//# MeqUVW.h: Class to calculate the UVW coordinates
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

#if !defined(MNS_MEQUVW_H)
#define MNS_MEQUVW_H

//# Includes
#include <MNS/MeqResult.h>
#include <MNS/MeqRequestId.h>

//# Forward Declarations
class MVBaseline;
class MeqRequest;


class MeqUVW
{
public:
  // Create with the given ra and dec of the phase reference position.
  MeqUVW (double ra, double dec);

  // Calculate the UVW coordinates for the given domain and interferometer.
  void calcUVW (const MeqRequest&, const MVBaseline&);

  // Get the u, v and w.
  const MeqResult& getU() const
    { return itsU; }
  const MeqResult& getV() const
    { return itsV; }
  const MeqResult& getW() const
    { return itsW; }

private:
  double itsRa;
  double itsDec;
  double itsRaCen;
  double itsSinDec;
  double itsCosDec;
  double itsSinRac;
  double itsCosRac;
  MeqRequestId itsLastRequest;
  MeqResult itsU;
  MeqResult itsV;
  MeqResult itsW;
};


#endif
