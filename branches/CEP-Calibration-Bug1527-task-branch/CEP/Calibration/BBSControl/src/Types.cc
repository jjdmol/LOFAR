//#  Types.cc:
//#
//# Copyright (C) 2002-2007
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

#include <BBSControl/Types.h>
#include <BBSControl/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    void fromParameterSet(const ParameterSet &ps, Selection &selection)
    {
      // Read baseline type.
      selection.type = ps.getString("Selection.BaselineType", selection.type);

      // Read baseline selection.
      selection.baselines.clear();
      if(ps.isDefined("Selection.Baselines"))
      {
        ParameterValue value(ps.get("Selection.Baselines"));
        ASSERTSTR(value.isVector(), "Error parsing baseline selection: "
            << value << "; expected a list enclosed in brackets, e.g."
            " [5*,[CS0??LBA,6*]]");

        vector<ParameterValue> patterns(value.getVector());
        for(vector<ParameterValue>::const_iterator pattern = patterns.begin(),
          pattern_end = patterns.end(); pattern != pattern_end; ++pattern)
        {
          vector<string> criterion(pattern->getStringVector());
          ASSERTSTR(criterion.size() == 1 || criterion.size() == 2, "Error"
            " parsing baseline selection criterion: " << criterion
            << "; expected a single shell style pattern, or a list enclosed in"
            " brackets that contains either one or two shell style patterns,"
            " e.g. 5*, [5*], or [5*,6*]");

          selection.baselines.push_back(criterion);
        }
      }

      selection.correlations = ps.getStringVector("Selection.Correlations",
        selection.correlations);
    }

    //# -------  ostream operators  ------- #//

    ostream& operator<<(ostream& os, const Selection &obj)
    {
      os << "Selection:";
      Indent id;
      os << endl << indent << "Baseline Type: " << obj.type
         << endl << indent << "Baselines: " << obj.baselines
         << endl << indent << "Correlations: " << obj.correlations;
      return os;
    }

    ostream& operator<<(ostream& os, const CellSize& obj)
    {
      os << "Cell size:";
      Indent id;
      os << endl << indent << "Frequency (channels): " << obj.freq
         << endl << indent << "Time (timestamps): " << obj.time;
      return os;
    }

  } // namespace BBS

} // namespace LOFAR
