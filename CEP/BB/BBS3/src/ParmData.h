//# ParmData.h: The properties for solvable parameters
//#
//# Copyright (C) 2004
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

#ifndef BB_BBS3_PARMDATA_H
#define BB_BBS3_PARMDATA_H

//# Includes
#include <BBS3/MNS/MeqMatrix.h>
#include <string>

namespace LOFAR {

//# Forward Declarations.
class BlobOStream;
class BlobIStream;

class ParmData
{
public:
  ParmData ();
  // Constructor.
  ParmData (const std::string& name, int nrSpid, int firstSpid,
	    const MeqMatrix& values);

  // Get the properties.
  // <group>
  const std::string& getName() const
    { return itsName; }
  int getNrSpid() const
    { return itsNrSpid; }
  int getFirstSpid() const
    { return itsFirstSpid; }
  const MeqMatrix& getValues() const
    { return itsValues; }
  // </group>

  // Write the object into a blob.
  friend BlobOStream& operator<< (BlobOStream&, const ParmData&);

  // Read the object from a blob.
  friend BlobIStream& operator>> (BlobIStream&, ParmData&);

private:
  int itsNrSpid;
  int itsFirstSpid;
  std::string itsName;
  MeqMatrix itsValues;
};


}

#endif
