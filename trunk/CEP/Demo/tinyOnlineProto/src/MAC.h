//# MAC.h: 
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

#ifndef ONLINEPROTO_MAC_H
#define ONLINEPROTO_MAC_H

#include <definitions.h>
#define pi 3.1415926535897932384626433832795028841972

#include <lofar_config.h>
#include <Common/Lorrays.h>
#include <tinyOnlineProto/Station.h>
//#include <iostream>

namespace LOFAR
{
class MAC
{
public:
   MAC ();
   ~MAC();
   MAC (const MAC& m); // copy constructor

   // Get/Set functions
   float getWe ();
   float getC ();
   float getDeclination ();
   void setDeclination (float d);
   float getStartHourangle (); 
   void setStartHourangle (float ha); 
   float getChannelBandwidth ();       
   void setChannelBandwidth (float cbw);       
   float getFrequency (int offset);
   int getBeamletSize ();
   void setBeamletSize (int bs);
   float getTotalBandwidth ();
   void setTotalBandwidth (float tbw);
   float getLOfrequency ();
   void setLOfrequency (float f_lo);
   Station* getStations ();
/*    void setStations (Station* s); */
   int getNumberOfStations ();
   void setNumberOfStations (int ns);
   int getNumberOfBeamlets ();
   void setNumberOfBeamlets (int nb);
   
 private:
   float w_e;                       // The rotational speed of the earth
   float c;                         // The speed of light
   
   // The pointing direction
   float itsDeclination;
   float itsStartHourangle;       // starting hourangle
   
   // Frequency related parameters
   float itsChannelBandwidth;       
   LoVec_float itsFrequencies;    // the center frequencies of every channel
   int itsBeamletSize;            // number channels per beamlet
   float itsTotalBandwidth;         // The total bandwidth after A/D conversion
   float itsLOfrequency;            // the local oscillator frequency
   int itsNumberOfBeamlets;
     
   // Station related parameters
   Station* itsStations[NSTATIONS];          // the stationpositions 
   int itsNumberOfStations;

   int itsDataIteration;
};


inline float MAC::getWe ()
  { return w_e; }
inline float MAC::getC ()
  { return c; }

inline float MAC::getDeclination ()
  { return itsDeclination; }

inline void MAC::setDeclination (float d)
  { itsDeclination = d; }

inline float MAC::getStartHourangle ()
  { return itsStartHourangle; }

inline void MAC::setStartHourangle (float ha)
  { itsStartHourangle = ha; }

inline float MAC::getChannelBandwidth ()
  { return itsChannelBandwidth; }

inline void MAC::setChannelBandwidth (float cbw)
  { itsChannelBandwidth = cbw; }

inline float MAC::getFrequency (int offset)
  { return itsFrequencies(offset); }

inline int MAC::getBeamletSize ()
  { return itsBeamletSize; }

inline void MAC::setBeamletSize (int bs)
  { itsBeamletSize = bs; }

inline float MAC::getTotalBandwidth ()
  { return itsTotalBandwidth; }

inline void MAC::setTotalBandwidth (float tbw)
  { itsTotalBandwidth = tbw; }

inline float MAC::getLOfrequency ()
  { return itsLOfrequency; }

inline void MAC::setLOfrequency (float f_lo)
  { itsLOfrequency = f_lo; }

inline Station* MAC::getStations ()
  { return itsStations[0]; }

/* inline void MAC::setStations (Station* s) */
/*   { itsStations = s; } */

inline int MAC::getNumberOfStations ()
  { return itsNumberOfStations; }

inline void MAC::setNumberOfStations (int ns)
  { itsNumberOfStations = ns; }

inline int MAC::getNumberOfBeamlets ()
  { return itsNumberOfBeamlets; }

inline void MAC::setNumberOfBeamlets (int nb)
  { itsNumberOfBeamlets = nb; }


} //end namespace LOFAR

#endif
