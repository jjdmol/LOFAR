//# NextChunkCommand.cc:
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/CommandVisitor.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <Common/ParameterSet.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

namespace LOFAR
{
  namespace BBS
  {

    // Register NextChunkCommand with the CommandFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = CommandFactory::instance().
        registerClass<NextChunkCommand>("nextchunk");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    CommandResult NextChunkCommand::accept(CommandVisitor &visitor) const
    {
      return visitor.visit(*this);
    }


    const string& NextChunkCommand::type() const
    {
      static const string theType("NextChunk");
      return theType;
    }


    void NextChunkCommand::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      Command::print(os);
      os << endl;
      os << "Frequency: " << setprecision(3) << itsFreqRange.first / 1e6
        << " - " << setprecision(3) << itsFreqRange.second / 1e6 << " MHz"
        << endl;
      os << "Time: "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(itsTimeRange.first, "s"))
        << " - "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(itsTimeRange.second, "s"))
        << endl;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void NextChunkCommand::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      ps.replace(KVpair("Freq.Start", itsFreqRange.first));
      ps.replace(KVpair("Freq.End", itsFreqRange.second));
      ps.replace(KVpair("Time.Start", itsTimeRange.first));
      ps.replace(KVpair("Time.End", itsTimeRange.second));
    }


    void NextChunkCommand::read(const ParameterSet& ps, const string prefix)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      itsFreqRange = make_pair(ps.getDouble(prefix+"Freq.Start"),
        ps.getDouble(prefix+"Freq.End"));
      itsTimeRange = make_pair(ps.getDouble(prefix+"Time.Start"),
        ps.getDouble(prefix+"Time.End"));
    }

  } //# namespace BBS

} //# namespace LOFAR
