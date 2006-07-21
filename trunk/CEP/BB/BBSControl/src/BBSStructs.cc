//#  BBSStructs.cc: 
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

#include <lofar_config.h>

#include <BBSControl/BBSStructs.h>
#include <BBSControl/StreamFormatting.h>

namespace LOFAR
{
  namespace BBS
  {

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
	 << endl << indent << "Local sky table: " << obj.localSky;
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


    ostream& operator<<(ostream& os, const Correlation& obj)
    {
      os << "Correlation:";
      Indent id;
      os << endl << indent << "Selection: ";
      switch(obj.selection) {
      case Correlation::NONE:  os << "NONE";  break;
      case Correlation::AUTO:  os << "AUTO";  break;
      case Correlation::CROSS: os << "CROSS"; break;
      case Correlation::ALL:   os << "ALL";   break;
      default: os << "*****"; break;
      }
      os << endl << indent << "Type:" << obj.type;
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


    ostream& operator<<(ostream& os, const Baselines& obj)
    {
      os << "Baselines:";
      Indent id;
      os << endl << indent << "Station1: " << obj.station1
	 << endl << indent << "Station2: " << obj.station2;
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
