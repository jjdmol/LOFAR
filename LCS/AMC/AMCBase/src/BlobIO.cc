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
#include <AMCBase/BlobIO.h>
#include <AMCBase/Coord3D.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Epoch.h>
#include <AMCBase/ConverterCommand.h>
#include <AMCBase/ConverterStatus.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h> 
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace AMC
  {

    //# -------  BlobOStream operators  ------- #//

    BlobOStream& operator<<(BlobOStream& bos, const Coord3D& coord)
    {
      bos << coord.get();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const Direction& dir)
    {
      bos << dir.coord()
          << static_cast<int32>(dir.type());
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const Position& pos)
    {
      bos << pos.coord();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const Epoch& epo)
    {
      bos << epo.getDay()
          << epo.getFraction();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const ConverterCommand& cc)
    {
      bos << static_cast<int32>(cc.get());
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const ConverterStatus& cs)
    {
      bos << static_cast<int32>(cs.get())
          << cs.text();
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const RequestData& req)
    {
      bos << req.direction
          << req.position
          << req.epoch;
      return bos;
    }

    BlobOStream& operator<<(BlobOStream& bos, const ResultData& res)
    {
      bos << res.direction;
      return bos;
    }


    //# -------  BlobIStream operators  ------- #//

    BlobIStream& operator>>(BlobIStream& bis, Coord3D& coord)
    {
      vector<double> xyz(3);
      bis >> xyz;
      coord = Coord3D(xyz);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, Direction& dir)
    {
      Coord3D coord;
      int32 type;
      bis >> coord >> type;
      dir = Direction(coord, static_cast<Direction::Types>(type));
      ASSERT(dir.isValid());
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, Position& pos)
    {
      Coord3D coord;
      bis >> coord;
      pos = Position(coord);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, Epoch& epo)
    {
      double day, fraction;
      bis >> day >> fraction;
      epo = Epoch(day, fraction);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, ConverterCommand& cc)
    {
      int32 cmd;
      bis >> cmd;
      cc = ConverterCommand(static_cast<ConverterCommand::Commands>(cmd));
      ASSERT(cc.isValid());
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, ConverterStatus& cs)
    {
      int32 sts;
      string txt;
      bis >> sts >> txt;
      cs = ConverterStatus(static_cast<ConverterStatus::Status>(sts), txt);
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, RequestData& req)
    {
      bis >> req.direction >> req.position >> req.epoch;
      return bis;
    }

    BlobIStream& operator>>(BlobIStream& bis, ResultData& res)
    {
      bis >> res.direction;
      return bis;
    }


  } // namespace AMC

} // namespace LOFAR
