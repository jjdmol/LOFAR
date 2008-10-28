//#  ParmDBMeta.cc: one line description
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

#include <lofar_config.h>

#include <ParmDB/ParmDBMeta.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


namespace LOFAR {
namespace BBS {

  ParmDBMeta::ParmDBMeta()
  {}

  ParmDBMeta::ParmDBMeta (const std::string& type,
                          const std::string& tableName)
    : itsType      (type),
      itsTableName (tableName)
  {}

  void ParmDBMeta::setSQLMeta (const std::string& dbName,
                               const std::string& userName,
                               const std::string& dbPwd,
                               const std::string& hostName)
  {
    itsDBName   = dbName;
    itsUserName = userName;
    itsDBPwd    = dbPwd;
    itsHostName = hostName;
  }

  BlobOStream& operator<< (BlobOStream& bos, const ParmDBMeta& pdm)
  {
    bos << pdm.itsType   << pdm.itsTableName
        << pdm.itsDBName << pdm.itsUserName
        << pdm.itsDBPwd  << pdm.itsHostName;
    return bos;
  }
    
  BlobIStream& operator>> (BlobIStream& bis, ParmDBMeta& pdm)
  {
    bis >> pdm.itsType >> pdm.itsTableName
        >> pdm.itsDBName >> pdm.itsUserName
        >> pdm.itsDBPwd >> pdm.itsHostName;
    return bis;
  }   

} // namespace BBS
} // namespace LOFAR
