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

#ifndef LOFAR_BB_BBS3_PARMDATA_H
#define LOFAR_BB_BBS3_PARMDATA_H

// \file ParmData.h
// The properties for solvable parameters

//# Includes
#include <BBS3/MNS/MeqMatrix.h>
#include <string>

namespace LOFAR {

// \addtogroup BBS3
// @{

//# Forward Declarations.
class BlobOStream;
class BlobIStream;

class ParmData
{
public:
  ParmData ();
  // Constructor.
  ParmData (const std::string& name,
	    const std::string& tableName,
	    const std::string& dbType,
	    const std::string& dbName,
	    int nrSpid, int firstSpid,
	    const MeqMatrix& values);

  // Get the properties.
  // <group>
  const std::string& getName() const
    { return itsName; }
  const std::string& getTableName() const
    { return itsTableName; }
  const std::string& getDBType() const
    { return itsDBType; }
  const std::string& getDBName() const
    { return itsDBName; }
  int getIndex() const
    { return itsTableIndex; }
  int getNrSpid() const
    { return itsNrSpid; }
  int getFirstSpid() const
    { return itsFirstSpid; }
  int getLastSpid() const
    { return itsFirstSpid + itsNrSpid - 1; }
  const MeqMatrix& getValues() const
    { return itsValues; }
  MeqMatrix& getRWValues()
    { return itsValues; }
  // </group>

  // Set the first spid.
  void setFirstSpid (int firstSpid)
    { itsFirstSpid = firstSpid; }

  // Update the values.
  void addValues (const double* values);

  // Check if two objects are equal.
  // They are if name, #spids and values are equal.
  bool operator== (const ParmData& other);

  // Write the object into a blob.
  friend BlobOStream& operator<< (BlobOStream&, const ParmData&);

  // Read the object from a blob.
  friend BlobIStream& operator>> (BlobIStream&, ParmData&);

  // Write the object.
  friend ostream& operator<< (ostream&, const ParmData&);

private:
  int itsNrSpid;
  int itsFirstSpid;
  int itsTableIndex;
  std::string itsName;
  std::string itsTableName;
  std::string itsDBType;
  std::string itsDBName;
  MeqMatrix itsValues;
};

// @}

}

#endif
