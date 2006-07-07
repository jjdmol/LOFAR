//# MeqParmSingle.cc: The class for a single parameter
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

#include <lofar_config.h>
#include <BBS3/MNS/MeqParmSingle.h>
#include <BBS3/MNS/MeqRequest.h>
#include <Common/lofar_vector.h>
#include <casa/Utilities/BinarySearch.h>
#include <cmath>

namespace LOFAR {

MeqParmSingle::MeqParmSingle (const string& name, MeqParmGroup* group,
			      double value)
: MeqParm         (name, group),
  itsInitialValue (value),
  itsCurValue     (value),
  itsSolveIndex   (-1)
{}

MeqParmSingle::~MeqParmSingle()
{}

int MeqParmSingle::initDomain (const MeqDomain&, int spidIndex)
{
  if (isSolvable()) {
    itsSolveIndex = spidIndex;
    return 1;
  } else {
    itsSolveIndex = -1;
  }
  return 0;
}

void MeqParmSingle::setValue (double value)
{
  itsCurValue = value;
  itsInitialValue = itsCurValue;
}

double MeqParmSingle::getValue() const
{
  return itsCurValue;
}

MeqResult MeqParmSingle::getResult (const MeqRequest& request)
{
  MeqResult result(request.nspid());
  result.setValue (MeqMatrix(itsCurValue));
  if (isSolvable()  &&  request.nspid() > 0) {
    double perturbation = 1e-6;
    if (std::abs(itsCurValue) > 1e-10) {
      perturbation *= itsCurValue;
    }
    result.setPerturbedValue (itsSolveIndex,
			     MeqMatrix(itsCurValue + perturbation));
    result.setPerturbation (itsSolveIndex, perturbation);
  }
  return result;
}

void MeqParmSingle::getCurrentValue (MeqMatrix& value) const
{
  value = MeqMatrix(itsCurValue, 1, 1);
}

void MeqParmSingle::update (const MeqMatrix& value)
{
  if (isSolvable()) {
    itsCurValue = value.getDouble (itsSolveIndex, 0);
  }
}

void MeqParmSingle::save()
{
  itsInitialValue = itsCurValue;
}

}
