//#  ABSSubbandStatsPSet.h: Property set for the subband statistics.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef ABSSUBBANDSTATSPSET_H_
#define ABSSUBBANDSTATSPSET_H_

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>

//#define WAIT_FOR_INPUT

#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)
// property sets

// echoPingPSET: start

namespace ABS
{

const TProperty BeamServerProps[] =
{
    // which subband to generate statistics for
    {"subband", GCFPValue::LPT_UNSIGNED, GCF_WRITABLE_PROP, "0" },

    // power level in the subband
    {"power_re", GCFPValue::LPT_DOUBLE, GCF_READABLE_PROP, "0"},
    {"power_im", GCFPValue::LPT_DOUBLE, GCF_READABLE_PROP, "0"},

    // seqnr of the last statistics package included in the stats
    {"seqnr", GCFPValue::LPT_UNSIGNED, GCF_READABLE_PROP, "0"},
};

const TPropertySet BeamServerPSet = 
{
    2, "BeamServer", BeamServerProps
};

};

#endif

