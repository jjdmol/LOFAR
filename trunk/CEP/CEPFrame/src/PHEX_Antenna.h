//# PHEX_Antenna.h:
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

#ifndef CEPFRAME_PHEX_ANTENNA_H
#define CEPFRAME_PHEX_ANTENNA_H

#include <lofar_config.h>

#include "CEPFrame/ParamHolder.h"
#include <math.h>

namespace LOFAR
{

/**
   This is the ParameterHolder for the example Antenna class; each antenna
   will get an instance of the class. 
 */
class PHEX_Antenna: ParamHolder
{
  public:

  /// Standard Parameter type class
  class DataType
    { public:
        float Xpos;
	float Ypos;
    };

  /// Constructor.
  PHEX_Antenna();

  /// Set the antenna position.
  void setPosition (float aXpos, float aYpos);

  /// Get the dish radius.
  float getRadius() const;

  /// Get the angle.
  float getAngle() const;

  /// Dump the contents of the object on stdout.
  void dump() const;

  /// Get the size of the data packet.
  int getDataPacketSize() const;

 private:
  /// Forbid copy constructor.
  PHEX_Antenna (const PHEX_Antenna&);
  /// Forbid assignment.
  PHEX_Antenna& operator= (const PHEX_Antenna&);


  DataType itsDataPacket;
};


inline int PHEX_Antenna::getDataPacketSize() const
{ return sizeof(DataType); }

inline float PHEX_Antenna::getRadius() const
{ return sqrt(itsDataPacket.Xpos * itsDataPacket.Xpos + 
              itsDataPacket.Ypos * itsDataPacket.Ypos); }

inline float PHEX_Antenna::getAngle() const
{ return atan2(itsDataPacket.Ypos, itsDataPacket.Xpos); }

}

#endif 
