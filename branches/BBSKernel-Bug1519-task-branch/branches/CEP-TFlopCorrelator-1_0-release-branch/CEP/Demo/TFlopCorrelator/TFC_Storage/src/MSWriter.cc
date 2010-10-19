// MSWriter.cc: interface class for filling a MeasurementSet
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
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <TFC_Storage/MSWriter.h>
#include <TFC_Storage/MSWriterImpl.h>
#include <casa/Exceptions/Error.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace casa;

MSWriter::MSWriter (const char* msName, double startTime, double timeStep, 
		    uint nantennas, const vector<double>& antPos)
: itsWriter (0)
{
  ASSERTSTR(antPos.size() == 3*nantennas, 
	    "Antenna position vector does not have the right size!");
  try {
    itsWriter = new MSWriterImpl (msName, startTime, timeStep, nantennas, 
				  antPos);
  } catch (AipsError x) {
    cerr << "MSWriter exception: " << x.getMesg() << endl;
    exit(0);
  } catch (...) {
    cerr << "Unexpected MSWriter exception" << endl;
    exit(0);
  }
}

MSWriter::~MSWriter()
{
  delete itsWriter;
}

int MSWriter::addBand (int npolarizations, int nchannels,
		       double refFreq, double chanWidth)
{
  return itsWriter->addBand (npolarizations, nchannels, refFreq, chanWidth);
}

int MSWriter::addBand (int npolarizations, int nchannels,
		       double refFreq, const double* chanFreqs,
		       const double* chanWidths)
{
  return itsWriter->addBand (npolarizations, nchannels, refFreq,
			     chanFreqs, chanWidths);
}

int MSWriter::addField (double azimuth, double elevation)
{
  return itsWriter->addField (azimuth, elevation);
}

void MSWriter::write (int& rowNr, int bandId, int fieldId, int channelId, 
		      int timeCounter, int nrdata, const fcomplex* data, 
		      const bool* flags)
{
  itsWriter->write (rowNr, bandId, fieldId, channelId, timeCounter, nrdata,
		    data, flags);
}

int MSWriter::nrAntennas() const
  { return itsWriter->nrAntennas(); }

int MSWriter::nrBands() const
  { return itsWriter->nrBands(); }

int MSWriter::nrFields() const
  { return itsWriter->nrFields(); }

int MSWriter::nrPolarizations() const
  { return itsWriter->nrPolarizations(); }

int MSWriter::nrTimes() const
  { return itsWriter->nrTimes(); }
