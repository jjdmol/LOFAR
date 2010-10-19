//#  RequestData.h: Request data that will be sent to the converter
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

#ifndef LOFAR_AMCBASE_REQUESTDATA_H
#define LOFAR_AMCBASE_REQUESTDATA_H

// \file
// Request data that will be sent to the converter

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {
    // \addtogroup AMCBase
    // @{

    // Struct wrapping the request data that will be sent to the converter.
    // The purpose of this struct is to hide the exact structure of the data
    // to be sent to the converter. It is not really necessary to make the
    // data members private, since we need to have easy access to the data
    // members. However, note that, because the data members are public, they
    // become part of the interface! So, be careful in renaming them. A number
    // of constructors have been defined in order to ease the creation of a
    // %RequestData instance.
    struct RequestData
    {
      RequestData() 
      {}

      RequestData(const SkyCoord& sc,
                  const EarthCoord& ec,
                  const TimeCoord& tc) :
        skyCoord(1, sc), earthCoord(1, ec), timeCoord(1, tc)
      {}        

      RequestData(const vector<SkyCoord>& sc,
                  const EarthCoord& ec,
                  const TimeCoord& tc) :
        skyCoord(sc), earthCoord(1, ec), timeCoord(1, tc)
      {}

      RequestData(const SkyCoord& sc,
                  const vector<EarthCoord>& ec,
                  const TimeCoord& tc) :
        skyCoord(1, sc), earthCoord(ec), timeCoord(1, tc)
      {}

      RequestData(const SkyCoord& sc,
                  const EarthCoord& ec,
                  const vector<TimeCoord>& tc) :
        skyCoord(1, sc), earthCoord(1, ec), timeCoord(tc)
      {}

      RequestData(const vector<SkyCoord>& sc,
                  const vector<EarthCoord>& ec,
                  const TimeCoord& tc) :
        skyCoord(sc), earthCoord(ec), timeCoord(1, tc)
      {}

      RequestData(const vector<SkyCoord>& sc,
                  const EarthCoord& ec,
                  const vector<TimeCoord>& tc) :
        skyCoord(sc), earthCoord(1, ec), timeCoord(tc)
      {}

      RequestData(const SkyCoord& sc,
                  const vector<EarthCoord>& ec,
                  const vector<TimeCoord>& tc) :
        skyCoord(1, sc), earthCoord(ec), timeCoord(tc)
      {}

      RequestData(const vector<SkyCoord>& sc,
                  const vector<EarthCoord>& ec, 
                  const vector<TimeCoord>& tc) :
        skyCoord(sc), earthCoord(ec), timeCoord(tc)
      {}

      vector<SkyCoord>   skyCoord;
      vector<EarthCoord> earthCoord;
      vector<TimeCoord>  timeCoord;
    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
