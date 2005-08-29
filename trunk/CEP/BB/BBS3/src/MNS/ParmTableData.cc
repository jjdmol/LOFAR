//#  ParmTableData.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <BBS3/MNS/ParmTableData.h>
#include <APS/ParameterSet.h>

namespace LOFAR {

    ParmTableData::ParmTableData()
      : itsMasterPort(0),
	itsIsMaster(false)
    {}

    ParmTableData::ParmTableData(const string& nameKey, const ACC::APS::ParameterSet& ps)
      : //itsName(name),
	itsMasterPort(0),
	itsIsMaster(false)
    {
      ASSERTSTR(ps.isDefined("DBType"), "DBType is not defined in the ParameterSet");
      itsType = ps.getString("DBType");
      ASSERTSTR(ps.isDefined(nameKey), nameKey << " is not defined in the ParameterSet");
      itsTableName = ps.getString(nameKey);

      if (itsType == "aips") {
	// do nothing, we have all the info we need

      } else if (itsType == "bdb") {
	// do nothing, we have all the info we need

      } else if (itsType == "bdbrepl") {
	ASSERTSTR(ps.isDefined("DBMasterHost"), "For bdbrepl DBMasterHost should be defined in the ParameterSet");
	itsHostName = ps.getString("DBMasterHost");
	ASSERTSTR(ps.isDefined("DBMasterPort"), "For bdbrepl DBMasterPort should be defined in the ParameterSet");
	itsMasterPort = ps.getInt32("DBMasterPort");
	ASSERTSTR(ps.isDefined("DBIsMaster"), "For bdbrepl DBIsMaster should be defined in the ParameterSet");
	itsIsMaster = ps.getBool("DBIsMaster");

      } else {
	// extract all information for other types
	ASSERTSTR(ps.isDefined("DBName"), "DBName is not defined in the ParameterSet");
	itsDBName = ps.getString("DBName");
	ASSERTSTR(ps.isDefined("DBUserName"), "DBUserName is not defined in the ParameterSet");
	itsUserName = ps.getString("DBUserName");
	ASSERTSTR(ps.isDefined("DBPwd"), "DBPwd is not defined in the ParameterSet");
	itsDBPwd = ps.getString("DBPwd");
	ASSERTSTR(ps.isDefined("DBMasterHost"), "DBMasterHost is not defined in the ParameterSet");
	itsHostName = ps.getString("DBMasterHost");
	ASSERTSTR(ps.isDefined("DBMasterPort"), "DBMasterPort is not defined in the ParameterSet");
	itsMasterPort = ps.getInt32("DBMasterPort");
	ASSERTSTR(ps.isDefined("DBIsMaster"), "DBIsMaster is not defined in the ParameterSet");
	itsIsMaster = ps.getBool("DBIsMaster");
      }
    }

    ParmTableData::~ParmTableData()
    {}

    ParmTableData::ParmTableData(const ParmTableData& that)
    {
      //      itsName = that.itsName;
      itsType = that.itsType;
      itsTableName = that.itsTableName;      
      itsDBName = that.itsDBName;
      itsUserName = that.itsUserName;
      itsDBPwd = that.itsDBPwd;
      itsHostName = that.itsHostName;
      itsMasterPort = that.itsMasterPort;
      itsIsMaster = that.itsIsMaster;
    }

    BlobOStream& operator<< (BlobOStream& bos, const ParmTableData& ptd)
    {
      bos //<< ptd.itsName 
	  << ptd.itsType << ptd.itsTableName
	  << ptd.itsDBName << ptd.itsUserName
	  << ptd.itsDBPwd << ptd.itsHostName
	  << ptd.itsMasterPort << ptd.itsIsMaster;
      return bos;
    }
    
    // Read the object from a blob.
    BlobIStream& operator>> (BlobIStream& bis, ParmTableData& ptd)
    {
      bis //>> ptd.itsName 
	  >> ptd.itsType >> ptd.itsTableName
	  >> ptd.itsDBName >> ptd.itsUserName
	  >> ptd.itsDBPwd >> ptd.itsHostName
	  >> ptd.itsMasterPort >> ptd.itsIsMaster;
      return bis;
    }   

} // namespace LOFAR
