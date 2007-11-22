//# Stepprop.cc: Calibration step properties
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

#include <BBSKernel/StepProp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS 
{

// \addtogroup BBS
// @{

void StepProp::setBaselines (const vector<int>& ant1Nrs,
			     const vector<int>& ant2Nrs)
{
  ASSERT (ant1Nrs.size() == ant2Nrs.size());
  itsAnt1 = ant1Nrs;
  itsAnt2 = ant2Nrs;
}

void StepProp::setBaselines (const vector<vector<int> >& ant1Groups,
			     const vector<vector<int> >& ant2Groups)
{
  ASSERT (ant1Groups.size() == ant2Groups.size());
  itsAnt1.clear();
  itsAnt2.clear();
  // Combine the antennae in each group of ant1 and ant2.
  vector<vector<int> >::const_iterator iter1;
  vector<vector<int> >::const_iterator iter2;
  for (iter1 = ant1Groups.begin(), iter2 = ant2Groups.begin();
       iter1 != ant1Groups.end();
       iter1++, iter2++) {
    formBaselines (*iter1, *iter2);
  }
}

void StepProp::formBaselines (const vector<int>& ant1,
			      const vector<int>& ant2) const
{
  for (vector<int>::const_iterator a1 = ant1.begin();
       a1 != ant1.end();
       a1++) {
    for (vector<int>::const_iterator a2 = ant2.begin();
	 a2 != ant2.end();
	 a2++) {
      itsAnt1.push_back (*a1);
      itsAnt2.push_back (*a2);
    }
  }
}

void StepProp::setBaselines (const vector<string>& ant1NamePatterns,
			     const vector<string>& ant2NamePatterns)
{
  ASSERT (ant1NamePatterns.size() == ant2NamePatterns.size());
  itsAnt1Regex.clear();
  itsAnt2Regex.clear();
  // Convert the patterns to proper Regex-es.
  vector<string>::const_iterator iter1;
  vector<string>::const_iterator iter2;
  for (iter1 = ant1NamePatterns.begin(), iter2 = ant2NamePatterns.begin();
       iter1 != ant1NamePatterns.end();
       iter1++, iter2++) {
    itsAnt1Regex.push_back (Regex::fromPattern (*iter1));
    itsAnt2Regex.push_back (Regex::fromPattern (*iter2));
  }
}

void StepProp::expandPatterns (const vector<string>& antNames) const
{
  if (! itsAnt1Regex.empty()) {
    itsAnt1.clear();
    itsAnt2.clear();
    // Loop through all Regex-es.
    vector<int> ant1;
    vector<int> ant2;
    vector<Regex>::const_iterator iter1;
    vector<Regex>::const_iterator iter2;
    for (iter1 = itsAnt1Regex.begin(), iter2 = itsAnt2Regex.begin();
	 iter1 != itsAnt1Regex.end();
	 iter1++, iter2++) {
      ant1.clear();
      ant2.clear();
      // If a names matches, add its antennanr to the vector.
      for (uint i=0; i<antNames.size(); ++i) {
	String name = antNames[i];
	if (name.matches (*iter1)) {
	  ant1.push_back (i);
	}
	if (name.matches (*iter2)) {
	  ant2.push_back (i);
	}
      }
      // Add all baselines formed by this group.
      formBaselines (ant1, ant2);
    }
    // If no baselines selected, add a non-existing one.
    // (because an empty vector means all).
    if (itsAnt1.size() == 0) {
      itsAnt1.push_back (antNames.size());
      itsAnt2.push_back (antNames.size());
    }
  }
}

} // namespace BBS
} // namespace LOFAR
