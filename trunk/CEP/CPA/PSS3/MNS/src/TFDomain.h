//# TFDomain.h: The domain for an expression
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

#if !defined(MNS_TFDOMAIN_H)
#define MNS_TFDOMAIN_H


// This class represents a domain for which an expression has to be
// evaluated.

class TFDomain
{
public:
  // Create a time,frequency domain with the given bin sizes.
  TFDomain (double startX, double endX, double stepX,
	    double startY, double endY, double stepY);

  // Get the start, end, and step of the domain.
  double startX() const
    { return itsStartX; }
  double endX() const
    { return itsEndX; }
  double stepX() const
    { return itsStepX; }
  double startY() const
    { return itsStartY; }
  double endY() const
    { return itsEndY; }
  double stepY() const
    { return itsStepY; }

  // Get the number of intervals in the domain.
  int nx() const
    { return itsNx; }
  int ny() const
    { return itsNy; }

  // Get the number of cells in the domain.
  int ncells() const
    { return itsNx * itsNy; }

private:
  double itsStartX;
  double itsEndX;
  double itsStepX;
  double itsStartY;
  double itsEndY;
  double itsStepY;
  int itsNx;
  int itsNy;
};


#endif
