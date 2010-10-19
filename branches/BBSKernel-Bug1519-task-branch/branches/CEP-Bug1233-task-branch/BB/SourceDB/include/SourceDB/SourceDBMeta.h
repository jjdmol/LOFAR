//#  SourceDBMeta.h: one line description
//#
//#  Copyright (C) 2008
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

#ifndef LOFAR_SOURCEDB_SOURCEDBMETA_H
#define LOFAR_SOURCEDB_SOURCEDBMETA_H

// \file
// one line description.

#include <Common/lofar_string.h>

namespace LOFAR {
//# Forward Declarations.
class BlobOStream;
class BlobIStream;

namespace SourceDB {

  // \addtogroup SourceDB
  // @{

  // Description of class.
  class SourceDBMeta
  {
  public:
    SourceDBMeta();

    SourceDBMeta (const std::string& type, const std::string& tableName);

    void setSQLMeta (const std::string& dbName, const std::string& userName,
		     const std::string& dbPwd, const std::string& hostName);

    const std::string& getType() const
      { return itsType; }

    const std::string& getTableName() const
      { return itsTableName; }

    const std::string& getDBName() const
      { return itsDBName; }
      
    const std::string& getUserName() const
      { return itsUserName; }
      
    const std::string& getDBPwd() const
      { return itsDBPwd; }
    
    const std::string& getHostName() const
      { return itsHostName; }
    
    // Write the object into a blob.
    friend BlobOStream& operator<< (BlobOStream&, const SourceDBMeta&);

    // Read the object from a blob.
    friend BlobIStream& operator>> (BlobIStream&, SourceDBMeta&);

  private:
    //# Datamembers
    std::string itsType;
    std::string itsTableName;
    // these options are used for sql databases
    std::string itsDBName;
    std::string itsUserName;
    std::string itsDBPwd;
    std::string itsHostName;
  };

  // @}

} // namespace SourceDB
} // namespace LOFAR

#endif
