//#  BlobIO.cc: implementation of Blob I/O stream operators
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <AMCBase/AMCClient/ConverterCommand.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>

namespace LOFAR
{
  namespace AMC
  {

    BlobOStream& operator<<(BlobOStream& bos, const SkyCoord& sc)
    {
      bos << sc.angle0()
          << sc.angle1();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const EarthCoord& ec)
    {
      bos << ec.longitude()
          << ec.latitude()
          << ec.height();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const TimeCoord& tc)
    {
      bos << tc.getDay()
          << tc.getFraction();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const ConverterCommand& cc)
    {
      bos << cc.get();
      return bos;
    }


    BlobIStream& operator>>(BlobIStream& bis, SkyCoord& sc)
    {
      double angle0, angle1;
      bis >> angle0 >> angle1;
      sc = SkyCoord(angle0, angle1);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, EarthCoord& ec)
    {
      double longitude, latitude, height;
      bis >> longitude >> latitude >> height;
      ec = EarthCoord(longitude, latitude, height);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, TimeCoord& tc)
    {
      double day, fraction;
      bis >> day >> fraction;
      tc = TimeCoord(day, fraction);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, ConverterCommand& cc)
    {
      int32 iCmd;
      bis >> iCmd;
      cc = ConverterCommand(iCmd);
      return bis;
    }

  } // namespace AMC

} // namespace LOFAR
