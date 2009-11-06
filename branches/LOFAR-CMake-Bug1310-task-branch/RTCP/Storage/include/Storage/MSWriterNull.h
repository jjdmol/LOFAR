//  MSMriterNull.h: null implementation of MSWriter
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: MSWriterImpl.h 11891 2008-10-14 13:43:51Z gels $
//
//////////////////////////////////////////////////////////////////////


#ifndef LOFAR_STORAGE_MSWRITERNULL_H
#define LOFAR_STORAGE_MSWRITERNULL_H


//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

#include <Storage/MSWriter.h>

//# Forward declarations

namespace LOFAR
{

  namespace RTCP
  {
    class MSWriterNull : public MSWriter
    {
    public:
      MSWriterNull(const char* msName, double startTime, double timeStep, 
		   int nfreq, int ncorr, int nantennas, const vector<double>& antPos, 
		   const vector<std::string>& storageStationNames, float weightFactor);
      ~MSWriterNull();

      int addBand(int, int, double, double);
      int addBand(int, int, double, const double*, const double*);
      void addField(double, double, unsigned);
      void write(int, int, int, StreamableData*);


      inline int nrAntennas() const
      { return itsNrAnt; }
      
      inline int nrBands() const
      { return itsNrBand; }

      inline int nrFields() const
      { return itsNrField; }
      
      inline int nrPolarizations() const
      { return itsNrPol; }

      inline int nrTimes() const
      { return itsNrTimes; }


    private:
      int itsNrBand;
      int itsNrField;
      int itsNrAnt;
      int itsNrFreq;
      int itsNrCorr;
      int itsNrTimes;
      int itsNrPol;
      int itsNrChan;
    };
  }
}

#endif
