//# MeqPointSource.h: Abstract base class for holding a source
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

#ifndef MNS_MEQSOURCE_H
#define MNS_MEQSOURCE_H

// \file
// Abstract base class for holding a source

//# Includes
#include <BBSKernel/MNS/MeqExpr.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

class MeqSource
{
public:
  // The default constructor.
  MeqSource();

  MeqSource (const string& name, const string& groupName,
         const MeqExpr& ra, const MeqExpr& dec);

  virtual ~MeqSource();

  const string& getName() const
    { return itsName; }

  const string& getGroupName() const
    { return itsGroupName; }

  // Get the source nr.
  int getSourceNr() const
    { return itsSourceNr; }

  // Set the source nr.
  void setSourceNr (int sourceNr)
    { itsSourceNr = sourceNr; }

  MeqExpr& getRa()
    { return itsRa; }
  MeqExpr& getDec()
    { return itsDec; }

protected:
  int     itsSourceNr;
  string  itsName;
  string  itsGroupName;
  MeqExpr itsRa;
  MeqExpr itsDec;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
