//#  BBSStructs.cc: 
//#
//#  Copyright (C) 2002-2007
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

#include <lofar_config.h>

#include <BBSControl/BBSStructs.h>
#include <Common/StreamUtil.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    //# -------  ostream operators  ------- #//

    ostream& operator<<(ostream& os, const BBDB& obj)
    {
      os << "Blackboard database:";
      Indent id;
      os << endl << indent << "Host: " << obj.host
	 << endl << indent << "Port: " << obj.port
	 << endl << indent << "DBName: " << obj.dbName
	 << endl << indent << "Username: " << obj.username
	 << endl << indent << "Password: " << obj.password;
      return os;
    }


    ostream& operator<<(ostream& os, const ParmDB& obj)
    {
      os << "Parameter database:";
      Indent id;
      os << endl << indent << "Instrument table: " << obj.instrument
	 << endl << indent << "Local sky table: " << obj.localSky
	 << endl << indent << "History table: " << obj.history;
      return os;
    }


    ostream& operator<<(ostream& os, const RegionOfInterest& obj)
    {
      os << "Region of interest:";
      Indent id;
      os << endl << indent << "Frequency: " << obj.frequency
      << endl << indent << "Time: " << obj.time;
      return os;
    }
    
    
    ostream& operator<<(ostream& os, const DomainSize& obj)
    {
      os << "Domain size:";
      Indent id;
      os << endl << indent << "Bandwidth: " << obj.bandWidth << " (Hz)"
	 << endl << indent << "Time interval: " << obj.timeInterval << " (s)";
      return os;
    }


    ostream& operator<<(ostream& os, const Integration& obj)
    {
      os << "Integration:";
      Indent id;
      os << endl << indent << "Delta frequency: " << obj.deltaFreq << " (Hz)"
	 << endl << indent << "Delta time: " << obj.deltaTime << " (s)";
      return os;
    }


    //# -------  BlobOStream operators  ------- #//

    BlobOStream& operator<<(BlobOStream& bos, const BBDB& obj)
    {
      bos << obj.host
	  << obj.port
	  << obj.dbName
	  << obj.username
	  << obj.password;
      return bos;
    }


    BlobOStream& operator<<(BlobOStream& bos, const ParmDB& obj)
    {
      bos << obj.instrument
	  << obj.localSky
	  << obj.history;
      return bos;
    }


    BlobOStream& operator<<(BlobOStream& bos, const RegionOfInterest& obj)
    {
      bos << obj.frequency
	  << obj.time;
      return bos;
    }
    
    
    BlobOStream& operator<<(BlobOStream& bos, const DomainSize& obj)
    {
      bos << obj.bandWidth
	  << obj.timeInterval;
      return bos;
    }


    BlobOStream& operator<<(BlobOStream& bos, const Integration& obj)
    {
      bos << obj.deltaFreq
	  << obj.deltaTime;
      return bos;
    }


    //# -------  BlobIStream operators  ------- #//

    BlobIStream& operator>>(BlobIStream& bis, BBDB& obj)
    {
      bis >> obj.host
	  >> obj.port
	  >> obj.dbName
	  >> obj.username
	  >> obj.password;
      return bis;
    }


    BlobIStream& operator>>(BlobIStream& bis, ParmDB& obj)
    {
      bis >> obj.instrument
	  >> obj.localSky
	  >> obj.history;
      return bis;
    }


    BlobIStream& operator>>(BlobIStream& bis, RegionOfInterest& obj)
    {
      bis >> obj.frequency
	  >> obj.time;
      return bis;
    }
    
    
    BlobIStream& operator>>(BlobIStream& bis, DomainSize& obj)
    {
      bis >> obj.bandWidth
	  >> obj.timeInterval;
      return bis;
    }


    BlobIStream& operator>>(BlobIStream& bis, Integration& obj)
    {
      bis >> obj.deltaFreq
	  >> obj.deltaTime;
      return bis;
    }


  } // namespace BBS

} // namespace LOFAR
