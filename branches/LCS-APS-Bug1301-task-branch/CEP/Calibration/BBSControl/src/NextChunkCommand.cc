//# NextChunkCommand.cc: 
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

    void NextChunkCommand::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
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


    void NextChunkCommand::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      
      itsFreqRange = make_pair(ps.getDouble("Freq.Start", 0.0),
          ps.getDouble("Freq.End", 0.0));
      itsTimeRange = make_pair(ps.getDouble("Time.Start", 0.0),
          ps.getDouble("Time.End", 0.0));
    }

  } //# namespace BBS

} //# namespace LOFAR
