//# MeqPointSource.cc: Abstract base class for a list of sources
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

#include <MNS/MeqPointSource.h>


MeqPointSource::MeqPointSource()
: itsI   (0),
  itsQ   (0),
  itsU   (0),
  itsV   (0),
  itsRa  (0),
  itsDec (0)
{}

MeqPointSource::MeqPointSource (MeqExpr* fluxI, MeqExpr* fluxQ,
				MeqExpr* fluxU, MeqExpr* fluxV,
				MeqExpr* ra, MeqExpr* dec)
: itsI   (fluxI),
  itsQ   (fluxQ),
  itsU   (fluxU),
  itsV   (fluxV),
  itsRa  (ra),
  itsDec (dec)
{}
