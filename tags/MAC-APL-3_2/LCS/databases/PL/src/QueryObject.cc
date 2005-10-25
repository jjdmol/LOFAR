//#  QueryObject.cc: implementation of the QueryObject class
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

#include <PL/QueryObject.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_sstream.h>

namespace LOFAR
{
  namespace PL
  {

    bool QueryObject::isEmpty() const
    {
      if (itsUseString) return itsSqlString.empty();
      else return itsQueryExpr.isNull();
    }

    string QueryObject::getSql() const 
    {
      LOG_TRACE_VAR_STR(__PRETTY_FUNCTION__ << ": itsUseString = "
               << (itsUseString ? "true" : "false"));
      if (isEmpty()) return "";
      if (itsUseString) {
        return "WHERE " + itsSqlString;
      }
      else {
        ostringstream oss;
        oss << "WHERE " << itsQueryExpr;
        LOG_TRACE_VAR_STR(__PRETTY_FUNCTION__ << ": return value : " 
                          << oss.str());
        return oss.str();
      }
    }

  } // namespace PL

} // namespace LOFAR
