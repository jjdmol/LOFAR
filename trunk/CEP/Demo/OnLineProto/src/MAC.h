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

#include <lofar_config.h>
#include <Common/Lorrays.h>
#include "OnLineProto/Station.h"

namespace LOFAR
{
class MAC
{
public:
   MAC ();
   ~MAC();
   MAC (const MAC& m); // copy constructor

   // Get/Set functions
   int getWe ();
   int getC ();
   int getIntegrationTime ();
   void setIntegrationTime (int t);
   float getDeclination ();
   void setDeclination (int d);
   float getStartHourangle (); 
   void setStartHourangle (int ha); 
   int getChannelBandwidth ();       
   void setChannelBandwidth (int cbw);       
   LoVec_float getFrequencies ();
   void setFrequencies (LoVec_float freqs);
   int getBeamletSize ();
   void setBeamletSize (int bs);
   int getTotalBandwidth ();
   void setTotalBandwidth (int tbw);
   int getLOfrequency ();
   void setLOfrequency (int f_lo);
   Station* getStations ();
   void setStations (Station* s);
   int getNumberOfStations ();
   void setNumberOfStations (int ns);
   int getNumberOfBeamlets ();
   void setNumberOfBeamlets (int nb);
   
 private:
   int w_e;                       // The rotational speed of the earth
   int c;                         // The speed of light
   int itsIntegrationTime;        // the correlator integration time
   
   // The pointing direction
   float itsDeclination;
   float itsStartHourangle;       // starting hourangle
   
   // Frequency related parameters
   int itsChannelBandwidth;       
   LoVec_float itsFrequencies;    // the center frequencies of every channel
   int itsBeamletSize;            // number channels per beamlet
   int itsTotalBandwidth;         // The total bandwidth after A/D conversion
   int itsLOfrequency;            // the local oscillator frequency
   int itsNumberOfBeamlets;
     
   // Station related parameters
   Station* itsStations;          // the stationpositions 
   int itsNumberOfStations;
};


inline int MAC::getWe ()
  { return w_e; }
inline int MAC::getC ()
  { return c; }

inline int MAC::getIntegrationTime ()
  { return itsIntegrationTime; }

inline void MAC::setIntegrationTime (int t)
  { itsIntegrationTime = t; }

inline float MAC::getDeclination ()
  { return itsDeclination; }

inline void MAC::setDeclination (int d)
  { itsDeclination = d; }

inline float MAC::getStartHourangle ()
  { return itsStartHourangle; }

inline void MAC::setStartHourangle (int ha)
  { itsStartHourangle = ha; }

inline int MAC::getChannelBandwidth ()
  { return itsChannelBandwidth; }

inline void MAC::setChannelBandwidth (int cbw)
  { itsChannelBandwidth = cbw; }

inline LoVec_float MAC::getFrequencies ()
  { return itsFrequencies; }

inline void MAC::setFrequencies (LoVec_float freqs)
  { itsFrequencies = freqs; }

inline int MAC::getBeamletSize ()
  { return itsBeamletSize; }

inline void MAC::setBeamletSize (int bs)
  { itsBeamletSize = bs; }

inline int MAC::getTotalBandwidth ()
  { return itsTotalBandwidth; }

inline void MAC::setTotalBandwidth (int tbw)
  { itsTotalBandwidth = tbw; }

inline int MAC::getLOfrequency ()
  { return itsLOfrequency; }

inline void MAC::setLOfrequency (int f_lo)
  { itsLOfrequency = f_lo; }

inline Station* MAC::getStations ()
  { return itsStations; }

inline void MAC::setStations (Station* s)
  { itsStations = s; }

inline int MAC::getNumberOfStations ()
  { return itsNumberOfStations; }

inline void MAC::setNumberOfStations (int ns)
  { itsNumberOfStations = ns; }

inline int MAC::getNumberOfBeamlets ()
  { return itsNumberOfStations; }

inline void MAC::setNumberOfBeamlets (int nb)
  { itsNumberOfBeamlets = nb; }

} //end namespace LOFAR

#endif
