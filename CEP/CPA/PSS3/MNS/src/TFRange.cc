//# TFRange.cc: The range of an expression for a domain.
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

//# Includes
#include <MNS/TFRange.h>
#include <MNS/MnsMatrixTmp.h>

TFRangeRep::TFRangeRep (int nspid)
: itsCount           (0),
  itsPerturbedValues (nspid),
  itsPerturbation    (nspid)
{}

TFRangeRep::~TFRangeRep()
{
  for (unsigned int i=0; i<itsPerturbedValues.size(); i++) {
    delete itsPerturbedValues[i];
  }
}

void TFRangeRep::unlink (TFRangeRep* rep)
{
  if (rep != 0  &&  --rep->itsCount == 0) {
    delete rep;
  }
}

void TFRangeRep::setValue (const MnsMatrix& value)
{
  itsValue = value;
}
  
void TFRangeRep::setPerturbedValue (int i, const MnsMatrix& value)
{
  if (itsPerturbedValues[i] == 0) {
    itsPerturbedValues[i] = new MnsMatrix();
  }
  *(itsPerturbedValues[i]) = value;
}

void TFRangeRep::show (ostream& os) const
{
  os << "Value: " << itsValue << endl;
  for (unsigned int i=0; i<itsPerturbedValues.size(); i++) {
    if (isDefined(i)) {
      os << "deriv parm " << i << " with " << itsPerturbation[i] << endl;
      os << "   " << (*(itsPerturbedValues[i]) - itsValue) /
	 MnsMatrix(itsPerturbation[i]) << endl;
    }
  }
}




TFRange::TFRange (int nspid)
{
  itsRep = new TFRangeRep(nspid);
  itsRep->link();
}

TFRange::TFRange (const TFRange& that)
: itsRep (that.itsRep)
{
  if (itsRep != 0) {
    itsRep->link();
  }
}
TFRange& TFRange::operator= (const TFRange& that)
{
  TFRangeRep::unlink (itsRep);
  itsRep = that.itsRep;
  if (itsRep != 0) {
    itsRep->link();
  }
  return *this;
}
