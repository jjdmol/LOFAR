//# ParmTable.h: Object to hold parameters in a table.
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

#if !defined(MNS_PARMTABLE_H)
#define MNS_PARMTABLE_H

// \file MNS/ParmTable.h
// Object to hold parameters in a table.

//# Includes
#include <BBS3/MNS/MeqPolc.h>
#include <BBS3/MNS/MeqSourceList.h>
#include <BBS3/MNS/ParmTableData.h>
#include <Common/lofar_vector.h>

//# Forward Declarations
namespace casa {
  template<class T> class Vector;
}
namespace LOFAR {
  class MeqDomain;
  class MeqParmGroup;
}

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

class ParmTableRep
{
public:
  ParmTableRep()
    {}

  virtual ~ParmTableRep()
    {}

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  virtual vector<MeqPolc> getPolcs (const string& parmName,
				    const MeqDomain& domain) = 0;

  // Get the initial polynomial coefficients for the given parameter.
  virtual MeqPolc getInitCoeff (const string& parmName) = 0;

  // Put the polynomial coefficient for the given parameter and domain.
  // If it is a new domain, the polc ID might be set.
  virtual void putCoeff (const string& parmName, MeqPolc& polc) = 0;

  // Put for a new parameter.
  virtual void putNewCoeff (const string& parmName, MeqPolc& polc) = 0;

  // Put the default coefficients.
  // The polc ID might be set.
  virtual void putDefCoeff (const string& parmName, MeqPolc& polc) = 0;

  // Put for a new default parameter.
  virtual void putNewDefCoeff (const string& parmName, MeqPolc& polc) = 0;

  // Get the names of all sources in the table.
  virtual vector<string> getSources() = 0;

  // Unlock the underlying table.
  virtual void unlock() = 0;

  // Connect to the database or table
  virtual void connect() = 0;

  // Create the database or table
  //  This has now become a static function and so it can't be overloaded.
  //  When creating a new ParmTable class, you can implement this function 
  //  and use it in parmdb just like with the other ParmTables.
  //virtual void createTable() = 0;
  // clear database or table
  virtual void clearTable() = 0;

  // Set or get the name and type.
  // <group>
  void setParmTableData(const ParmTableData& ptd)
    { itsPTD = ptd;};
  const ParmTableData& getParmTableData() const
    { return itsPTD; };
  // </group>
private:
  ParmTableData itsPTD;
};



class ParmTable
{
public:
  // Create the ParmTable object.
  // The dbType argument gives the database type.
  // otherwise a database of the given type.
  ParmTable (const ParmTableData& ptd);

  ~ParmTable()
    { delete itsRep; }

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  vector<MeqPolc> getPolcs (const string& parmName,
			    const MeqDomain& domain)
    { return itsRep->getPolcs (parmName, domain); }

  // Get the initial polynomial coefficients for the given parameter.
  MeqPolc getInitCoeff (const string& parmName)
    { return itsRep->getInitCoeff (parmName); }

  // Put the polynomial coefficient for the given parameter and domain.
  // If it is a new domain, the polc ID might be set.
  void putCoeff (const string& parmName, MeqPolc& polc)
    { itsRep->putCoeff (parmName, polc); }

  // Return point sources for the given source numbers.
  // An empty sourceNr vector means all sources.
  // In the 2nd version the pointers to the created MeqParm objects
  // are added to the vector of objects to be deleted.
  // <group>
  MeqSourceList getPointSources (MeqParmGroup*,
				 const casa::Vector<int>& sourceNrs);
  // </group>

  // Unlock the underlying table.
  void unlock()
    { itsRep->unlock(); }

  // Get the name and type.
  // <group>
  const ParmTableData& getParmTableData() const
    { return itsRep->getParmTableData(); }

private:
  // Forbid copy and assignment.
  ParmTable (const ParmTable&);
  ParmTable& operator= (const ParmTable&);

  ParmTableRep* itsRep;

};

// @}

}

#endif
