//  PHEX_Antenna.h:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.6  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.5  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.4  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.3  2001/02/05 14:53:05  loose
//  Added GPL headers
//

// PHEX_Antenna
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_PHEX_ANTENNA_H
#define BASESIM_PHEX_ANTENNA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/ParamHolder.h"
#include <math.h>

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


#endif 
