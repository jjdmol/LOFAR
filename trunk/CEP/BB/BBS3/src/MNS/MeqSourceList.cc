//# MeqSourceList.cc: List of sources
//#
//# Copyright (C) 2002
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

#include <BBS3/MNS/MeqSourceList.h>
#include <Common/LofarLogger.h>


namespace LOFAR {

void MeqSourceList::add (const MeqPointSource& source)
{
  itsSelected.push_back (itsSources.size());
  itsSources.push_back (source);
}

void MeqSourceList::setSelected (const vector<int>& sel)
{
  if (sel.size() == 0) {
    itsSelected.resize (itsSources.size());
    for (unsigned int i=0; i<itsSources.size(); i++) {
      itsSelected[i] = i;
    }
  } else {
    for (unsigned int i=0; i<sel.size(); i++) {
      ASSERT (sel[i] >= 0);
      ASSERT (sel[i] < int(itsSources.size()));
    }
    itsSelected = sel;
  }
}

}
