//# ParmTableDB.h: Object to hold parameters in a database table.
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

#if !defined(MNS_PARMTABLEDB_H)
#define MNS_PARMTABLEDB_H

//# Includes
#include <MNS/ParmTable.h>
#include <MNS/MeqPolc.h>
#include <Common/lofar_vector.h>

#include <lofar_config.h>

#ifdef HAVE_LOFAR_PL
# include <PL/PersistenceBroker.h>
# include <MNS/TPOParm.h>

typedef LOFAR::PL::TPersistentObject<MeqParmHolder> TPOMParm;
typedef LOFAR::PL::Collection<TPOMParm> MParmSet;
typedef LOFAR::PL::TPersistentObject<MeqParmDefHolder> TPOMParmDef;
typedef LOFAR::PL::Collection<TPOMParmDef> MParmDefSet;
#endif

//# Forward Declarations
class MeqDomain;


class ParmTableDB : public ParmTableRep
{
public:
  // Create the ParmTable object.
  // The dbType argument gives the database type.
  // It can be postgres.
  ParmTableDB (const string& dbType, const string& tableName,
	       const string& username, const string& passwd);

  virtual ~ParmTableDB();

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  virtual vector<MeqPolc> getPolcs (const string& parmName,
				    int sourceNr, int station,
				    const MeqDomain& domain);

  // Get the initial polynomial coefficients for the given parameter.
  virtual MeqPolc getInitCoeff (const string& parmName,
				int sourceNr, int station);

  // Put the polynomial coefficient for the given parameter and domain.
  virtual void putCoeff (const string& parmName,
			 int sourceNr, int station,
			 const MeqPolc& polc);

  // Get the names of all sources in the table.
  virtual vector<string> getSources();

  // Unlock the underlying table.
  virtual void unlock();

private:
#ifdef HAVE_LOFAR_PL
  // Find the table subset containing the parameter values for the
  // requested domain.
  MParmSet find (const string& parmName, 
		 int sourceNr, int station,
		 const MeqDomain& domain);

  LOFAR::PL::PersistenceBroker itsBroker;
#endif
  string                       itsTableName;
};


#endif
