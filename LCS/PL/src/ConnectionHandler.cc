//#  ConnectionHandler.cc: implementation of the database connection handler
//#
//#  Copyright (C) 2002-2003
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

#include <PL/ConnectionHandler.h>
#include <PL/Exception.h>
#include <dtl/DBConnection.h>
#include <dtl/DBException.h>
#include <sstream>

namespace LCS
{
  namespace PL
  {

    using namespace dtl;
    using namespace std;

    void ConnectionHandler::connect(const string& aDsn, 
                                    const string& aUid, 
                                    const string& aPwd)
    {
      try {
        ostringstream oss;
        oss << "DSN=" << aDsn << ";UID=" << aUid << ";PWD=" << aPwd << ";";
        DBConnection::GetDefaultConnection().Connect(oss.str());
      }
      catch (DBException& e) {
        THROW(PLException,"Failed to connect to database.\n" << e.what());
      }
    }
    
  } // namespace PL

} // namespace LCS
