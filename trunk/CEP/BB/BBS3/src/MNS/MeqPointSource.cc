//# MeqPointSource.cc: Class holding the expressions defining a point source
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

#include <lofar_config.h>
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqPointSource.h>


namespace LOFAR {

MeqPointSource::MeqPointSource()
: itsSourceNr (-1),
  itsGroupNr  (-1)
{}

MeqPointSource::MeqPointSource (const string& name,
				const MeqExpr& fluxI, const MeqExpr& fluxQ,
				const MeqExpr& fluxU, const MeqExpr& fluxV,
				const MeqExpr& ra, const MeqExpr& dec)
: itsSourceNr (-1),
  itsGroupNr  (-1),
  itsName     (name),
  itsI        (fluxI),
  itsQ        (fluxQ),
  itsU        (fluxU),
  itsV        (fluxV),
  itsRa       (ra),
  itsDec      (dec)
{}

}
