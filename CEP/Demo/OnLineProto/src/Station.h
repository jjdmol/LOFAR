//# Station.h: 
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

#ifndef ONLINEPROTO_STATION_H
#define ONLINEPROTO_STATION_H

#include <lofar_config.h>
#include <Common/Lorrays.h>

namespace LOFAR
{
class Station
{
public:
   Station (int ID, float x, float y, float z);
   ~Station();
   Station (const Station& s); // copy constructor

   // Get/Set functions
   float getX ();
   float getY ();
   float getZ ();
   int getID ();
   
 private:
   float itsX;  // the x position
   float itsY;  // the y position
   float itsZ;  
   int itsID;   // an unique identifier
};

inline float Station::getX ()
  { return itsX; }

inline float Station::getY ()
  { return itsY; }

inline float Station::getZ ()
  { return itsZ; }

inline int Station::getID ()
  { return itsID; }

} //end namespace LOFAR

#endif
