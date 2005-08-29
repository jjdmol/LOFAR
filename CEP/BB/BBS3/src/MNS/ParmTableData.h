//#  ParmTableData.h: one line description
//#
//#  Copyright (C) 2005
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

#ifndef LOFAR_BBS3MNS_PARMTABLEDATA_H
#define LOFAR_BBS3MNS_PARMTABLEDATA_H

// \file MNS/ParmTableData.h
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <APS/ParameterSet.h>

namespace LOFAR {
class BlobOStream;
class BlobIStream;
}

namespace LOFAR 
{

    // \addtogroup BBS3
    // @{

    //# Forward Declarations
    //class forward;


    // Description of class.
    class ParmTableData
    {
    public:
      ParmTableData(const string& nameKey, const ACC::APS::ParameterSet& ps);
      ParmTableData(const ParmTableData& that);
      ParmTableData();
      ~ParmTableData();      

      // Write the object into a blob.
      friend BlobOStream& operator<< (BlobOStream&, const ParmTableData&);

      // Read the object from a blob.
      friend BlobIStream& operator>> (BlobIStream&, ParmTableData&);

      const string& getName() const {return itsTableName;};
    private:
      // the ParmTable is allowed to read these members
      friend class ParmTable;
      //      friend class ParmTableRep;
      friend class ParmData;

      //# Datamembers
      //      string itsName;
      string itsType;
      string itsTableName;
      // these options are used for sql databases
      string itsDBName;
      string itsUserName;
      string itsDBPwd;
      // this one is used for sql and BDBReplication
      string itsHostName;
      // these are used for BDBReplication
      int itsMasterPort;
      bool itsIsMaster;
    };

    // @}

} // namespace LOFAR

#endif
