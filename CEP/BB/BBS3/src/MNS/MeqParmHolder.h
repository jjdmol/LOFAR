//# MeqParmHolder.h: Holder containing a shared pointer to a MeqStoredParmPolc
//#
//# Copyright (C) 2003
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

#ifndef MNS_MEQPARMHOLDER_H
#define MNS_MEQPARMHOLDER_H

//# Includes
#include <Common/lofar_string.h>
#include <BBS3/MNS/MeqPolc.h>


namespace LOFAR {

class MeqParmDefHolder
{
public:
  MeqParmDefHolder()
    : itsSrcnr(0), itsStatnr(0)
    {}

  MeqParmDefHolder (const string& name, int srcnr, int statnr,
		    const MeqPolc& polc)
    : itsName(name), itsSrcnr(srcnr), itsStatnr(statnr), itsPolc(polc)
    {}

  const string& getName() const
    { return itsName; }

  void setName (const string& name)
    { itsName = name; }

  int getSourceNr() const
    { return itsSrcnr; }

  void setSourceNr (int srcnr)
    { itsSrcnr = srcnr; }

  int getStation() const
    { return itsStatnr; }

  void setStation (int statnr)
    { itsStatnr = statnr; }

  const MeqPolc& getPolc() const
    { return itsPolc; }

  void setPolc (const MeqPolc& polc)
    { itsPolc = polc; }

  void setDomain (const MeqDomain& domain)
    { itsPolc.setDomain (domain); }

private:
  string  itsName;
  int     itsSrcnr;
  int     itsStatnr;
  MeqPolc itsPolc;
};


class MeqParmHolder : public MeqParmDefHolder
{
public:
  MeqParmHolder()
    {}

  MeqParmHolder (const string& name, int srcnr, int statnr,
		 const MeqPolc& polc)
    : MeqParmDefHolder (name, srcnr, statnr, polc)
    {}
};

}

#endif
