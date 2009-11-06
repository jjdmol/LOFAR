//# MSWriterNull: a null MSWriter
//#
//#  Copyright (C) 2001
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
//#  $Id: $

#include <lofar_config.h>

#include <AMCBase/Epoch.h>
#include <Common/LofarLogger.h>
#include <Storage/MSWriter.h>
#include <Storage/MSWriterFile.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif


namespace LOFAR 
{

  namespace RTCP
  {

     MSWriterFile::MSWriterFile (const char*msName, double , double ,
                                int nfreq, int ncorr, int nantennas, const vector<double>& ,
				const vector<string>&, float)
       : itsNrBand           (0),
	 itsNrField          (0),
	 itsNrAnt            (nantennas),
	 itsNrFreq           (nfreq), 
	 itsNrCorr           (ncorr),
	 itsNrTimes          (0),
	 itsNrPol            (0),
	 itsNrChan           (0),
         itsFile             (msName,00666)
     {
     }

    MSWriterFile::~MSWriterFile()
    {
    }

    int MSWriterFile::addBand(int, int, double, double)
    {
      itsNrBand++;
      return itsNrBand;
    }

    int MSWriterFile::addBand(int, int, double, const double*, const double*)
    {
      itsNrBand++;
      return itsNrBand;
    }

    void MSWriterFile::addField(double, double, unsigned)
    {
      itsNrField++;
    }

    void MSWriterFile::write(int, int, int, StreamableData *data)
    {
      data->write( &itsFile, true );
    }


  } // namespace RTCP
} // namespace LOFAR

