///# DHGrowStrategy.h: class that defines how the dataHolders grow.
//#
//# Copyright (C) 2000, 2001
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

#ifndef DHGROWSTRATEGY_HH
#define DHGROWSTRATEGY_HH
#include <CEPFrame/DataManager.h>
#include "P2Perf/DH_VarSize.h"

using namespace LOFAR;

class DHGrowStrategy
{
public:
  DHGrowStrategy ();
  
  void growDHs(TinyDataManager* DM);

protected:
  virtual void growDH(DH_VarSize* DH);
};

class ExpStrategy : public DHGrowStrategy
{
public:
  ExpStrategy (double factor);
  
protected:
  virtual void growDH(DH_VarSize* DH);
  double itsFactor;  
};

class LineairStrategy : public DHGrowStrategy
{
 public:
  LineairStrategy (int increment);

 protected:
  virtual void growDH(DH_VarSize* DH);
  int itsIncrement;
};

class MeasurementStrategy : public DHGrowStrategy
{
 public:
  MeasurementStrategy ();

 protected:
  virtual void growDH(DH_VarSize* DH);
};
#endif
