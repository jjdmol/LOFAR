//# Endian.h: Class to determine if the host uses little or big endian
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

#ifndef COORD_ENDIAN_H
#define COORD_ENDIAN_H

namespace LOFAR
{

  class Endian
  {
  public:
    // Determine if the machine uses big or little endian format.
    // (big endian is the SUN forat, little endian is the PC format).
    Endian();

    // Is the format little endian?
    int isLittleEndian() const
    { return itsLittle; }

    // Convert the data (in place) to the local format (by swapping the bytes).
    static void swap (int n, short* buf);
    static void swap (int n, int* buf);
    static void swap (int n, float* buf);
    static void swap (int n, double* buf);

    // Convert a single value.
    static short swapShort (const void* in);
    static int swapInt (const void* in);
    static float swapFloat (const void* in);
    static double swapDouble (const void* in);

  private:
    int itsLittle;
  };


  inline short Endian::swapShort (const void* in)
  {
    union {
      short val;
      char buf[2];
    } tmp;
    const char* inc = static_cast<const char*>(in);
    tmp.buf[1] = inc[0];
    tmp.buf[0] = inc[1];
    return tmp.val;
  }
  inline int Endian::swapInt (const void* in)
  {
    union {
      int val;
      char buf[4];
    } tmp;
    const char* inc = static_cast<const char*>(in);
    tmp.buf[3] = inc[0];
    tmp.buf[2] = inc[1];
    tmp.buf[1] = inc[2];
    tmp.buf[0] = inc[3];
    return tmp.val;
  }
  inline float Endian::swapFloat (const void* in)
  {
    union {
      float val;
      char buf[4];
    } tmp;
    const char* inc = static_cast<const char*>(in);
    tmp.buf[3] = inc[0];
    tmp.buf[2] = inc[1];
    tmp.buf[1] = inc[2];
    tmp.buf[0] = inc[3];
    return tmp.val;
  }
  inline double Endian::swapDouble (const void* in)
  {
    union {
      double val;
      char buf[8];
    } tmp;
    const char* inc = static_cast<const char*>(in);
    tmp.buf[7] = inc[0];
    tmp.buf[6] = inc[1];
    tmp.buf[5] = inc[2];
    tmp.buf[4] = inc[3];
    tmp.buf[3] = inc[4];
    tmp.buf[2] = inc[5];
    tmp.buf[1] = inc[6];
    tmp.buf[0] = inc[7];
    return tmp.val;
  }

} // namespace LOFAR

#endif
