//# MeqDomain.h: The domain for an expression
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

#if !defined(MNS_MEQDOMAIN_H)
#define MNS_MEQDOMAIN_H

// \file MNS/MeqDomain.h
// The domain for an expression

// This class represents a domain for which an expression has to be
// evaluated.
// The domain is normalized to the interval [-1,1].
//    offset = (st+end)/2    and    scale = (end-st)/2.
//    st = off-scale         and    end = off+scale
// Then
//    scaledvalue = (realvalue - offset) / scale


namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

class MeqDomain
{
public:
  // Create an  x,y default domain of -1:1,-1:1..
  MeqDomain();

  // Create an x,y domain.
  MeqDomain (double startX, double endX, double startY, double endY);

  // Get offset and scale value.
  double offsetX() const
    { return itsOffsetX; }
  double scaleX() const
    { return itsScaleX; }
  double offsetY() const
    { return itsOffsetY; }
  double scaleY() const
    { return itsScaleY; }

  // Transform a value to its normalized value.
  double normalizeX (double value) const
    { return (value - itsOffsetX) / itsScaleX; }
  double normalizeY (double value) const
    { return (value - itsOffsetY) / itsScaleY; }

  // Get the start, end, and step of the domain.
  double startX() const
    { return itsOffsetX - itsScaleX; }
  double endX() const
    { return itsOffsetX + itsScaleX; }
  double startY() const
    { return itsOffsetY - itsScaleY; }
  double endY() const
    { return itsOffsetY + itsScaleY; }

  bool operator== (const MeqDomain& that) const
  { return itsOffsetX == that.itsOffsetX  &&  itsScaleX == that.itsScaleX
       &&  itsOffsetY == that.itsOffsetY  &&  itsScaleY == that.itsScaleY; }

private:
  double itsOffsetX;
  double itsScaleX;
  double itsOffsetY;
  double itsScaleY;
};

// @}

}

#endif
