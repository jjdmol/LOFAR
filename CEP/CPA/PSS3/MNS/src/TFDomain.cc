//# TFDomain.cc: The domain for an expression
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

#include <MNS/TFDomain.h>
#include <Common/Debug.h>

TFDomain::TFDomain (double startX, double endX, double stepX,
		    double startY, double endY, double stepY)
: itsStartX (startX),
  itsEndX   (endX),
  itsStartY (startY),
  itsEndY   (endY)
{
  AssertMsg (startX < endX, "TFDomain: startX " << startX <<
	     " must be < endX " << endX);
  AssertMsg (startY < endY, "TFDomain: startY " << startY <<
	     " must be < endY " << endY);
  itsNx = int((itsEndX - itsStartX) / stepX + 0.5);
  if (itsNx == 0) {
    itsNx = 1;
  }
  itsStepX = (itsEndX - itsStartX) / itsNx;
  itsNy = int((itsEndY - itsStartY) / stepY + 0.5);
  if (itsNy == 0) {
    itsNy = 1;
  }
  itsStepY = (itsEndY - itsStartY) / itsNy;
  cout << startX << ' ' << itsStartX << endl;
  cout << endX << ' ' << itsEndX << endl;
  cout << stepX << ' ' << itsStepX << endl;
  cout << startY << ' ' << itsStartY << endl;
  cout << endY << ' ' << itsEndY << endl;
  cout << stepY << ' ' << itsStepY << endl;
  cout << itsNx << ' ' << itsNy << endl;
}
