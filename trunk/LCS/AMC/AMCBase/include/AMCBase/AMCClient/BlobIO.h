//#  BlobIO.h: one line description
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

#ifndef AMC_AMCBASE_BLOBIO_H
#define AMC_AMCBASE_BLOBIO_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes

namespace LOFAR
{
  //# Forward declarations
  class BlobOStream;
  class BlobIStream;

  namespace AMC
  {
    //# Forward declarations
    class SkyCoord;
    class EarthCoord;
    class TimeCoord;

    // Serialize the sky coordinates \a sc to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const SkyCoord& sc);

    // Serialize the earth coordinates \a ec to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const EarthCoord& ec);

    // Serialize the time coordinates \a tc to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const TimeCoord& tc);

    // De-serialize the input blob stream \a bis to the sky coordinates \a sc.
    BlobIStream& operator>>(BlobIStream& bis, SkyCoord& sc);

    // De-serialize the input blob stream \a bis to the earth coordinates \a
    // ec.
    BlobIStream& operator>>(BlobIStream& bis, EarthCoord& ec);

    // De-serialize the input blob stream \a bis to the time coordinates \a
    // tc.
    BlobIStream& operator>>(BlobIStream& bis, TimeCoord& tc);

  } // namespace AMC

} // namespace LOFAR

#endif
