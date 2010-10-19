//#  Signals.h: contains several signal generators
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_GENERATOR_SIGNALS_H
#define LOFAR_GENERATOR_SIGNALS_H

// \file
// Signal generators (several classes)

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Generator/RSPEthFrame.h>

namespace LOFAR 
{
  namespace Generator 
  {

    // Description of class.
    class Signal
    {
    public:
      typedef float SignalTime;
      Signal()
	{};
      virtual ~Signal() {};

      virtual void fillNext(Data* dataptr) = 0;

    private:
      // Copying is not allowed
      Signal (const Signal& that);
      Signal& operator= (const Signal& that);

      //# Datamembers
    };

    // This signal only produces zeros
    class Sig_Zero : public Signal
    {
    public:
      Sig_Zero()
	{};
      ~Sig_Zero() {};

      void fillNext(Data* dataptr);
    };

    // This signal only produces random numbers
    class Sig_Random : public Signal
    {
    public:
      Sig_Random()
	{};
      ~Sig_Random() {};

      int16 randint16() {
	int32 value = rand() - RAND_MAX / 2;  
	// we need to return a int16 with 12 used bits
	// so divide by 2^20
	return value / 1048576 ;
      };

      void fillNext(Data* dataptr);
    };

    // This signal produces the sum of several monochrome signals
    // It is only written in the first subband
    class Sig_MultiChrome : public Signal
    {
    public:
      struct SignalInfo{
      public:
	double amplitude;
	double frequency;
	double phase;
      };

      Sig_MultiChrome(vector<SignalInfo> info, double deltaT) : 
	itsSignals(info),
	itsDeltaPhase(2 * M_PI * deltaT)
	{};
      ~Sig_MultiChrome() {};

      void fillNext(Data* dataptr);

    private:
      vector<SignalInfo> itsSignals;
      double itsDeltaPhase;  // This is 2*pi*deltaT
    };

    // @}

  } // namespace Generator
} // namespace LOFAR

#endif
