//# Endian.cc: Class to determine if the host uses little or big endian
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

#include <Coord/Endian.h>

namespace LOFAR
{

  Endian::Endian()
  {
    union {
      int val;
      char buf[4];
    } tmp;
    tmp.val = 1;
    itsLittle = tmp.buf[0] == 1;
  }


  void Endian::swap (int n, short* buf)
  {
    for (int i=0; i<n; i++) {
      buf[i] = swapShort (buf+i);
    }
  }

  void Endian::swap (int n, int* buf)
  {
    for (int i=0; i<n; i++) {
      buf[i] = swapShort (buf+i);
    }
  }

  void Endian::swap (int n, float* buf)
  {
    for (int i=0; i<n; i++) {
      buf[i] = swapShort (buf+i);
    }
  }

  void Endian::swap (int n, double* buf)
  {
    for (int i=0; i<n; i++) {
      buf[i] = swapShort (buf+i);
    }
  }

} // namespace LOFAR
