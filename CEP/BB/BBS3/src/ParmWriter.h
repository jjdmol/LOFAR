//#  ParmWriter.h: Writes/updates parameters in ParmTables
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BB_BBS3_PARMWRITER_H
#define LOFAR_BB_BBS3_PARMWRITER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file ParmWriter.h
// Writes/updates parameters in ParmTables.

//# Includes
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  // \addtogroup BBS3
  // @{
  //# Forward Declarations
  class ParmData;
  class ParmTable;

  // Description of class.
  class ParmWriter
  {
  public:
    // Constructor
    ParmWriter();

    // Destructor
    ~ParmWriter();

    void useTable(const string& dbType, const string& tableName,
		  const string& dbName, const string& pwd,
		  const string& hostName);

    void write(vector<ParmData>& pData, double fStart, double fEnd, 
	       double tStart, double tEnd);

  private:
    typedef map<string, ParmTable*> ParmTableMap;

    ParmTableMap itsTables;     // Map containing used ParmTables

  };

  // @}
} // namespace LOFAR

#endif
