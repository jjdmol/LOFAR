///# DHGrowStrategy.cc: class that defines how the dataHolders grow.
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

#include <tinyCEP/TinyDataManager.h>
#include "P2Perf/DHGrowStrategy.h"

DHGrowStrategy::DHGrowStrategy ()
{};

void DHGrowStrategy::growDHs(TinyDataManager* DM)
{
  for (int i = 0; i < DM->getInputs(); i++)
    growDH((DH_VarSize*)DM->getInHolder(i));
  for (int i = 0; i < DM->getOutputs(); i++)
    growDH((DH_VarSize*)DM->getOutHolder(i));  
};

void DHGrowStrategy::growDH(DH_VarSize* DH)
{
};

ExpStrategy::ExpStrategy (double factor): itsFactor(factor)
{
};

void ExpStrategy::growDH(DH_VarSize* DH)
{
  DH->setSpoofedDataSize(itsFactor * DH->getDataSize());
};

LineairStrategy::LineairStrategy (int increment): itsIncrement(increment)
{};

void LineairStrategy::growDH(DH_VarSize* DH)
{
  DH->setSpoofedDataSize(DH->getDataSize() + itsIncrement);
};

MeasurementStrategy::MeasurementStrategy ()
{};

void MeasurementStrategy::growDH(DH_VarSize* DH)
{
  int size = DH->getDataSize();
  if (size < 5000 or size > 100000)
    // the packet size is outside of the interesting area for TH_Mem so take big steps
    size = 2 * size;
  else
    // the bandwidth is high here so take a lot of measurements
    size = 1.26 * size;

  DH->setSpoofedDataSize(size);
};


