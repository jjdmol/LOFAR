//# MeqSourceList.h: List of sources
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

#if !defined(MNS_MEQSOURCELIST_H)
#define MNS_MEQSOURCELIST_H

//# Includes
#include <BBS3/MNS/MeqPointSource.h>
#include <Common/lofar_vector.h>


namespace LOFAR {

class MeqSourceList
{
public:
  // The default constructor.
  MeqSourceList()
    {};

  // Add a source.
  void add (const MeqPointSource&);

  // Get the number of sources to be used.
  int size() const
    { return itsSelected.size(); }

  // Get the actual source number in the source list.
  int actualSourceNr (int sourceNr) const
    { return itsSelected[sourceNr]; }

  // Get the i-th selected source.
  MeqPointSource& operator[] (int i)
    { return itsSources[itsSelected[i]]; }

  // Set the sources to be actually used.
  // An empty vector selects all sources.
  void setSelected (const vector<int>&);
  
private:
  vector<MeqPointSource> itsSources;
  vector<int>            itsSelected;
};

}

#endif
