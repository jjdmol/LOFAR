//#  CommandQueueTransactors.cc: Transaction functors for the BBS command queue
//#
//#  Copyright (C) 2002-2007
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
#include <BBSControl/CommandQueueTransactors.h>
#include <BBSControl/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_typeinfo.h>

namespace LOFAR
{
  namespace BBS 
  {
    using LOFAR::operator<<;


    //##--------  ExecQuery  --------##//

    string ExecQuery::emptyString;

    ExecQuery::ExecQuery(const string& query) :
      pqxx::transactor<>("ExecQuery"),
      itsQuery(query),
      itsResult(emptyString)
    {
    }


    ExecQuery::ExecQuery(const string& query, string& result) :
      pqxx::transactor<>("ExecQuery"),
      itsQuery(query),
      itsResult(result)
    {
    }


    void ExecQuery::operator()(argument_type& transaction)
    {
      LOG_DEBUG_STR("Executing query : " << itsQuery);
      itsPQResult = transaction.exec(itsQuery);
    }


    void ExecQuery::on_commit()
    {
      ostringstream oss;
      uint rows(itsPQResult.size());
      uint cols(itsPQResult.columns());

      oss << "_nrows = " << rows << endl;
      for (uint row = 0; row < rows; ++row) {
        for (uint col = 0; col < cols; ++col) {
          oss << "_row(" << row << ")."
              << itsPQResult[row][col].name() << " = "
              << itsPQResult[row][col].c_str() << endl;
        }
      }
      itsResult = oss.str();
    }


  } // namespace BBS
  
} // namespace LOFAR
