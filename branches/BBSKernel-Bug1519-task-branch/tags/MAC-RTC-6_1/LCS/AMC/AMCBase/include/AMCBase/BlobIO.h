//#  BlobIO.h: Blob (de)serialization methods for AMCBase classes.
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

#ifndef LOFAR_AMCBASE_BLOBIO_H
#define LOFAR_AMCBASE_BLOBIO_H

// \file
// Blob (de)serialization methods for AMCBase classes.

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
    class Direction;
    class Position;
    class Epoch;
    class ConverterCommand;
    class ConverterStatus;
    struct RequestData;
    struct ResultData;

    // \addtogroup AMCBase
    // @{

    //# -------  BlobOStream operators  ------- #//

    // Serialize the sky coordinates \a dir to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const Direction& dir);

    // Serialize the earth coordinates \a pos to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const Position& pos);

    // Serialize the time coordinates \a epo to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const Epoch& epo);

    // Serialize the converter command \a cc to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const ConverterCommand& cc);

    // Serialize the return value \a cs to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const ConverterStatus& cs);

    // Serialize the converter request \a req to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const RequestData& req);

    // Serialize the converter result \a res to the output blob stream \a bos.
    BlobOStream& operator<<(BlobOStream& bos, const ResultData& res);


    //# -------  BlobIStream operators  ------- #//

    // De-serialize the input blob stream \a bis to the sky coordinates \a dir.
    BlobIStream& operator>>(BlobIStream& bis, Direction& dir);

    // De-serialize the input blob stream \a bis to the earth coordinates \a
    // pos.
    BlobIStream& operator>>(BlobIStream& bis, Position& pos);

    // De-serialize the input blob stream \a bis to the time coordinates \a
    // epo.
    BlobIStream& operator>>(BlobIStream& bis, Epoch& epo);

    // De-serialize the input blob stream \a bis to the converter command \a
    // cc.
    BlobIStream& operator>>(BlobIStream& bis, ConverterCommand& cc);

    // De-serialize the input blob stream \a bis to the return value \a cs.
    BlobIStream& operator>>(BlobIStream& bis, ConverterStatus& cs);

    // De-serialize the input blob stream \a bis to the converter request \a
    // req.
    BlobIStream& operator>>(BlobIStream& bis, RequestData& req);

    // De-serialize the input blob stream \a bis to the converter result \a
    // res.
    BlobIStream& operator>>(BlobIStream& bis, ResultData& res);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
