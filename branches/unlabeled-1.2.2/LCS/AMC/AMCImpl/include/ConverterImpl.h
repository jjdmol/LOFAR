//#  ConverterImpl.h: implementation of the Converter interface.
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_AMCIMPL_CONVERTERIMPL_H
#define LOFAR_AMCIMPL_CONVERTERIMPL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCBase/Converter.h>

namespace LOFAR
{
  namespace AMC
  {
    // This class represents the client side of the client/server
    // implementation of the AMC. It implements the Converter interface.
    class ConverterImpl : public Converter
    {
    public:
      ConverterImpl();

      virtual ~ConverterImpl() {}

      virtual SkyCoord j2000ToAzel(const SkyCoord& radec, 
                                   const EarthCoord& pos, 
                                   const TimeCoord& time);

      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
                                            const EarthCoord& pos,
                                            const TimeCoord& time);

      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
                                            const vector<EarthCoord>& pos,
                                            const TimeCoord& time);

      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
                                            const EarthCoord& pos,
                                            const vector<TimeCoord>& time);

      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time);

      virtual SkyCoord azelToJ2000 (const SkyCoord& azel,
                                    const EarthCoord& pos,
                                    const TimeCoord& time);

      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
                                            const EarthCoord& pos,
                                            const TimeCoord& time);

      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time);
    private:

    };

  } // namespace AMC
  
} // namespace LOFAR

#endif
