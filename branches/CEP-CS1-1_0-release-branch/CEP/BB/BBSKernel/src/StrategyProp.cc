//# Strategyprop.cc: Calibration strategy properties
//#
//# Copyright (C) 2006
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

#include <BBSKernel/StrategyProp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS 
{

// \addtogroup BBS
// @{

void StrategyProp::setAntennas (const vector<string>& antNamePatterns)
{
  itsAntRegex.clear();
  // Convert the patterns to proper Regex-es.
  for (vector<string>::const_iterator iter = antNamePatterns.begin();
       iter != antNamePatterns.end();
       iter++) {
    itsAntRegex.push_back (Regex::fromPattern (*iter));
  }
}

void StrategyProp::expandPatterns (const vector<string>& antNames) const
{
  if (! itsAntRegex.empty()) {
    itsAntNrs.clear();
    // Loop through all Regex-es.
    for (vector<Regex>::const_iterator iter = itsAntRegex.begin();
	 iter != itsAntRegex.end();
	 iter++) {
      // If a names matches, add its antennanr to the vector.
      for (uint i=0; i<antNames.size(); ++i) {
	String name (antNames[i]);
	if (name.matches (*iter)) {
	  itsAntNrs.push_back (i);
	}
      }
    }
  }
}

} // namespace BBS
} // namespace LOFAR
