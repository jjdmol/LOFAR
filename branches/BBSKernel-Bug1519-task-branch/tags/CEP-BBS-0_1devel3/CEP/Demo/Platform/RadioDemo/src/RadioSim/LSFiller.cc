// LSFiller.cc: interface class for filling a MeasurementSet for LofarSim
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
//  $Id$
//
//  $Log$
//  Revision 1.1  2001/03/29 11:24:38  gvd
//  Added classes to write an MS
//
//
//////////////////////////////////////////////////////////////////////


#include "LSFiller.h"
#include "LSFillerImpl.h"
#ifdef AIPSPP
# include <aips/Exceptions/Error.h>
#endif
#include <iostream>


LSFiller::LSFiller (const char* msName, double timeStep, int nantennas,
		    const float** antPos)
: itsFiller (0)
{
  // Form the antenna position vector.
  double* antPosOut = new double[3*nantennas];
  for (int i=0; i<nantennas; i++) {
    antPosOut[3*i + 0] = antPos[i][0];
    antPosOut[3*i + 1] = antPos[i][1];
    antPosOut[3*i + 2] = 0.;
  }
  try {
    itsFiller = new LSFillerImpl (msName, timeStep, nantennas, antPosOut);
    delete [] antPosOut;
#ifdef AIPSPP
  } catch (AipsError x) {
    cerr << "LSFiller exception: " << x.getMesg() << endl;
    exit(0);
#endif
  } catch (...) {
    cerr << "Unexpected LSFiller exception" << endl;
    exit(0);
  }
}

LSFiller::~LSFiller()
{
  delete itsFiller;
}

int LSFiller::addBand (int npolarizations, int nchannels,
		       double refFreq, double chanWidth)
{
  return itsFiller->addBand (npolarizations, nchannels, refFreq, chanWidth);
}

int LSFiller::addBand (int npolarizations, int nchannels,
		       double refFreq, const double* chanFreqs,
		       const double* chanWidths)
{
  return itsFiller->addBand (npolarizations, nchannels, refFreq,
			     chanFreqs, chanWidths);
}

int LSFiller::addField (double azimuth, double elevation)
{
  return itsFiller->addField (azimuth, elevation);
}

void LSFiller::write (int bandId, int fieldId, int timeCounter, int nrdata,
		      const complex<float>* data, const bool* flags)
{
  itsFiller->write (bandId, fieldId, timeCounter, nrdata,
		    data, flags);
}

int LSFiller::nrAntennas() const
  { return itsFiller->nrAntennas(); }

int LSFiller::nrBands() const
  { return itsFiller->nrBands(); }

int LSFiller::nrFields() const
  { return itsFiller->nrFields(); }

int LSFiller::nrPolarizations() const
  { return itsFiller->nrPolarizations(); }

int LSFiller::nrTimes() const
  { return itsFiller->nrTimes(); }
