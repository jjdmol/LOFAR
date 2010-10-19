//#  Flaggers.h: contains several flaggers
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

#ifndef LOFAR_GENERATOR_FLAGGERS_H
#define LOFAR_GENERATOR_FLAGGERS_H

// \file
// Several flaggers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Generator/RSPTimeStamp.h>
#include <bitset>

namespace LOFAR 
{
  namespace Generator 
  {

    // Description of class.
    class Flagger
    {
    public:
      Flagger(){};
      virtual ~Flagger() {};

      virtual bool sendData(const TimeStamp& t) {return true;};

    private:
      // Copying is not allowed
      Flagger& operator= (const Flagger& that);

      //# Datamembers
    };

    class RangeFlagger : public Flagger {
    public:
      RangeFlagger(long minBlockId, long maxBlockId) :
	itsMinBlockId(minBlockId),
	itsMaxBlockId(maxBlockId)
	{};
      virtual ~RangeFlagger(){};

      virtual bool sendData(const TimeStamp&t);
    private:
      long itsMinBlockId;
      long itsMaxBlockId;
    };

    // @}

  } // namespace Generator
} // namespace LOFAR

#endif
