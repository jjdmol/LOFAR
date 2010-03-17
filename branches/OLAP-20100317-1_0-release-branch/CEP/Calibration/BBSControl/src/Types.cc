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
#include <Common/lofar_sstream.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    //# -------  ostream operators  ------- #//

    ostream& operator<<(ostream& os, const Correlation& obj)
    {
      os << "Correlation:";
      Indent id;
      os << endl << indent << "Selection: " << obj.selection
         << endl << indent << "Type: "      << obj.type;
      return os;
    }


    ostream& operator<<(ostream& os, const Baselines& obj)
    {
      os << "Baselines:";
      Indent id;
      os << endl << indent << "Station1: " << obj.station1
         << endl << indent << "Station2: " << obj.station2;
      return os;
    }

    ostream& operator<<(ostream& os, const CellSize& obj)
    {
      os << "Cell size:";
      Indent id;
      os << endl << indent << "Frequency (channels): " << obj.freq << endl
         << indent << "Time (timestamps): " << obj.time;
      return os;
    }


    ostream& operator<<(ostream& os, const SolverOptions& obj)
    {
      os << "Solver options:";
      Indent id;
      os << endl << indent << "Max nr. of iterations: "  << obj.maxIter
         << endl << indent << "Epsilon value: "          << obj.epsValue
         << endl << indent << "Epsilon derivative: "     << obj.epsDerivative
         << endl << indent << "Colinearity factor: "     << obj.colFactor
         << endl << indent << "LM factor: "              << obj.lmFactor
         << boolalpha
         << endl << indent << "Balanced equations: "     << obj.balancedEqs
         << endl << indent << "Use SVD: "                << obj.useSVD
         << noboolalpha;
      return os;
    }

  } // namespace BBS

} // namespace LOFAR
