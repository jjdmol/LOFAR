//#  Structs.cc: 
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

#include <BBSControl/Structs.h>
#include <BBSControl/StreamUtil.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_iomanip.h>
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
      os << endl << indent << "Host: "     << obj.host
	 << endl << indent << "Port: "     << obj.port
	 << endl << indent << "DBName: "   << obj.dbName
	 << endl << indent << "Username: " << obj.username
	 << endl << indent << "Password: " << obj.password;
      return os;
    }


    ostream& operator<<(ostream& os, const ParmDB& obj)
    {
      os << "Parameter database:";
      Indent id;
      os << endl << indent << "Instrument table: " << obj.instrument
	 << endl << indent << "Local sky table: "  << obj.localSky
	 << endl << indent << "History table: "    << obj.history;
      return os;
    }


    ostream& operator<<(ostream& os, const RegionOfInterest& obj)
    {
      os << "Region of interest:";
      Indent id;
      os << endl << indent << "Frequency: " << obj.frequency
         << endl << indent << "Time: "      << obj.time;
      return os;
    }
    
    
    ostream& operator<<(ostream& os, const DomainSize& obj)
    {
      os << "Domain size:";
      Indent id;
      os << endl << indent << "Bandwidth: "     << obj.bandWidth    << " (Hz)"
	 << endl << indent << "Time interval: " << obj.timeInterval << " (s)";
      return os;
    }


    ostream& operator<<(ostream& os, const Integration& obj)
    {
      os << "Integration:";
      Indent id;
      os << endl << indent << "Delta frequency: " << obj.deltaFreq << " (Hz)"
	 << endl << indent << "Delta time: "      << obj.deltaTime << " (s)";
      return os;
    }


    ostream& operator<<(ostream& os, const SolverOptions& obj)
    {
      os << "Solver options:";
      Indent id;
      os << endl << indent << "Max nr. of iterations: "  << obj.maxIter
         << endl << indent << "Epsilon value: "          << obj.epsValue
         << endl << indent << "Epsilon derivative: "     << obj.epsDerivative
         << endl << indent << "Min fraction converged: " << obj.minConverged
         << endl << indent << "Collinearity factor: "    << obj.collFactor
         << endl << indent << "LM factor: "              << obj.lmFactor
         << boolalpha
         << endl << indent << "Balanced equations: "     << obj.balancedEqs
         << endl << indent << "Use SVD: "                << obj.useSVD
         << noboolalpha;
      return os;
    }

    ostream& operator<<(ostream& os, const Correlation& obj)
    {
      os << "Correlation:";
      Indent id;
      os << endl << indent << "Selection: " << obj.selection
	 << endl << indent << "Type: "      << obj.type;
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


    //# -------  BlobOStream operators  ------- #//

    BlobOStream& operator<<(BlobOStream& bos, const Correlation& obj)
    {
      bos << obj.selection
	  << obj.type;
      return bos;
    }


    BlobOStream& operator<<(BlobOStream& bos, const Baselines& obj)
    {
      bos << obj.station1
	  << obj.station2;
      return bos;
    }


    //# -------  BlobIStream operators  ------- #//

    BlobIStream& operator>>(BlobIStream& bis, Correlation& obj)
    {
      int32 selection;
      bis >> obj.selection;
      bis >> obj.type;
      return bis;
    }


    BlobIStream& operator>>(BlobIStream& bis, Baselines& obj)
    {
      bis >> obj.station1
	  >> obj.station2;
      return bis;
    }

  } // namespace BBS

} // namespace LOFAR
