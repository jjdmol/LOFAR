//#  PersistenceBroker.cc: implementation of persistence broker class.
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
#include <PL/PersistenceBroker.h>
#include <PL/PersistentObject.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace PL
  {

    void PersistenceBroker::connect(const string& aDsn, 
                                    const string& aUid, 
                                    const string& aPwd)
    {
      ConnectionHandler::connect(aDsn, aUid, aPwd);
    }
    

    void PersistenceBroker::erase(const PersistentObject& po) const
    {
      po.erase();
    }


    void PersistenceBroker::save(const PersistentObject& po,
                                 enum SaveMode sm) const
    {
      switch(sm) {
      case AUTOMATIC:
        po.save();
        break;
      case INSERT:
        po.insert();
        break;
      case UPDATE:
        po.update();
        break;
      default:
        THROW(BrokerException,"Invalid SaveMode");
      }
    }


  } // namespace PL

} // namespace LOFAR
