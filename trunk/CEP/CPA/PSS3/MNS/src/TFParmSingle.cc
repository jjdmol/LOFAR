//# TFParmSingle.cc: The class for a single parameter
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

#include <MNS/TFParmSingle.h>
#include <MNS/TFRequest.h>
#include <aips/Utilities/BinarySearch.h>

TFParmSingle::TFParmSingle (unsigned int type, double value)
: TFParm          (type),
  itsInitialValue (value),
  itsNewValue     (value),
  itsSolveIndex   (-1)
{}

void TFParmSingle::init (const TFDomain&)
{}

int TFParmSingle::setSolvable (int spidIndex)
{
  itsSolveIndex = spidIndex;
  return 1;
}

TFRange TFParmSingle::getRange (const TFRequest& request)
{
  TFRange range(request.nspid());
  range.setValue (itsNewValue);
  double perturbation = itsNewValue * 0.000001;
  range.setPerturbedValue (itsSolveIndex, itsNewValue + perturbation);
  range.setPerturbation (itsSolveIndex, perturbation);
  return range;
}

void TFParmSingle::update (const MnsMatrix& value)
{
  if (itsSolveIndex >= 0) {
    itsNewValue = value.getDouble (itsSolveIndex, 1);
  }
}

void TFParmSingle::save()
{
  itsInitialValue = itsNewValue;
}
