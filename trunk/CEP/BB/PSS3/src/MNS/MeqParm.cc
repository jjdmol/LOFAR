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

#include <PSS3/MNS/MeqParm.h>
#include <Common/Debug.h>


namespace LOFAR {

unsigned int MeqParm::theirNparm = 0;
vector<MeqParm*>* MeqParm::theirParms = 0;


MeqParm::MeqParm (const string& name)
: itsName       (name),
  itsIsSolvable (false)
{
  if (theirParms == 0) {
    theirParms = new vector<MeqParm*>;
  }
  if (theirNparm == theirParms->size()) {
    itsParmId = theirParms->size();
    theirParms->push_back (this);
  } else {
    Assert (theirNparm < theirParms->size());
    bool found = false;
    int cnt = 0;
    for (vector<MeqParm*>::iterator iter=theirParms->begin();
	 iter != theirParms->end();
	 iter++, cnt++) {
      if (*iter == 0) {
	itsParmId = cnt;
	*iter = this;
	found = true;
	break;
      }
    }
    Assert (found);
  }
  theirNparm++;
  Assert (theirNparm <= theirParms->size());
  Assert ((*theirParms)[itsParmId] == this);
}

MeqParm::~MeqParm()
{
  Assert (theirNparm <= theirParms->size());
  Assert ((*theirParms)[itsParmId] == this);
  (*theirParms)[itsParmId] = 0;
  theirNparm--;
}

const vector<MeqParm*>& MeqParm::getParmList()
{
  if (theirParms == 0) {
    theirParms = new vector<MeqParm*>;
  }
  return *theirParms;
}

void MeqParm::clearParmList()
{
  delete theirParms;
  theirParms = 0;
  theirNparm = 0;
}

}
