//# ParmTableBDB.h: Object to hold parameters in a database table.
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

#if !defined(MNS_PARMTABLEBDB_H)
#define MNS_PARMTABLEBDB_H

// \file MNS/ParmTableBDB.h
// Object to hold parameters in a database table.

//# Includes
#include <lofar_config.h>
#include <MNS/ParmTable.h>
#include <MNS/MeqParmHolder.h>
#include <MNS/MeqPolc.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <db_cxx.h>

//# Forward Declarations
namespace casa {
  template<class T> class Vector;
}
namespace LOFAR {
  class MeqDomain;
}

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

  class ParmTableBDB : public ParmTableRep{
  public:
    // Create the ParmTable object.
    // The dbType argument gives the database type.
    // It can be postgres.
    ParmTableBDB (const string& userName,
		  const string& tableName);

    virtual ~ParmTableBDB();

    // Get the parameter values for the given parameter and domain.
    // The matchDomain argument is set telling if the found parameter
    // matches the domain exactly.
    // Note that the requested domain may contain multiple polcs.
    virtual vector<MeqPolc> getPolcs (const string& parmName,
				      const MeqDomain& domain);

    // Get the initial polynomial coefficients for the given parameter.
    virtual MeqPolc getInitCoeff (const string& parmName);

    // Put the polynomial coefficient for the given parameter and domain.
    virtual void putCoeff (const string& parmName,
			   const MeqPolc& polc);

    // Put the default coefficients
    virtual void putDefCoeff (const string& parmName,
			      const MeqPolc& polc);

    virtual void putNewCoeff (const string& parmName,
			      const MeqPolc& polc);
    virtual void putNewDefCoeff (const string& parmName,
				 const MeqPolc& polc);

    // Get the names of all sources in the table.
    virtual vector<string> getSources();

    // Unlock the underlying table.
    virtual void unlock();

    // Connect to the database
    virtual void connect();
    // Create the database or table
    static void createTable(const string& userName, const string& tableName);
    // clear database or table
    virtual void clearTable();

  private:
    Db itsDb;

    vector<MeqParmHolder> find (const string& parmName, 
				const MeqDomain& domain);

    string itsTableName;
    // class used as the index for the database
    class MPHKey : public Dbt {
    public:
      MPHKey(string name, MeqDomain md);
      MPHKey(string name);
      MPHKey();
      void resetKey(string name, MeqDomain md);
      void resetKey(string name);
    protected:
      void updateThang();
      string itsName;
      MeqDomain itsDomain;
      char* itsBuffer;
    };
    // class used as the value for the database
    class MPHValue : public Dbt {
    public:
      MPHValue(MeqParmHolder mph);
      MPHValue();
      void resetMPH(MeqParmHolder mph);
      MeqParmHolder getMPH();
    protected:
      void updateThang();
      void updateFromThang();
      MeqParmHolder itsMPH;
      char* itsBuffer;
      MeqMatrix string2MeqMatrix(char* str, int length);
      void MeqMat2string(const MeqMatrix &MM, char* begin, char** end);
    };
    struct MPHData {
      double perturbation;
      double x0;
      double y0;
      MeqDomain domain;
      bool isRelPerturbation;
      unsigned int stringSizes[4];
    };
  };

  // @}

}

#endif
