//# MeqParm.cc: The base class for a parameter
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

#include <BBS3/MNS/MeqParm.h>
#include <Common/LofarLogger.h>


namespace LOFAR {


MeqParm::MeqParm (const string& name, MeqParmGroup* group)
: itsName       (name),
  itsIsSolvable (false),
  itsGroup      (group)
{
  itsParmId = itsGroup->add (this);
  DBGASSERT (itsGroup->getParm(itsParmId) == this);
}

MeqParm::~MeqParm()
{
  ASSERT (itsGroup->getParm(itsParmId) == this);
  itsGroup->remove (itsParmId);
}

string MeqParm::getTableName() const
{
  return "";
}
string MeqParm::getDBType() const
{
  return "";
}
string MeqParm::getDBName() const
{
  return "";
}


MeqParmGroup::MeqParmGroup()
  : itsNparm (0)
{}

int MeqParmGroup::add (MeqParm* parmPtr)
{
  int inx = 0;
  if (itsNparm == itsParms.size()) {
    inx = itsParms.size();
    itsParms.push_back (parmPtr);
  } else {
    ASSERT (itsNparm < itsParms.size());
    bool found = false;
    for (vector<MeqParm*>::iterator iter=itsParms.begin();
	 iter != itsParms.end();
	 iter++, inx++) {
      if (*iter == 0) {
	*iter = parmPtr;
	found = true;
	break;
      }
    }
    ASSERT (found);
  }
  itsNparm++;
  ASSERT (itsNparm <= itsParms.size());
  return inx;
}

void MeqParmGroup::remove (int index)
{
  ASSERT (itsNparm <= itsParms.size());
  itsParms[index] = 0;
  itsNparm--;
}

void MeqParmGroup::clear()
{
  for (uint i=0; i<itsParms.size(); ++i) {
    if (itsParms[i]) {
      // Note that the MeqParm destructor calls remove.
      delete itsParms[i];
    }
  }
  ASSERT (itsNparm == 0);
}

}
