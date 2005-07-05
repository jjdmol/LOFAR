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
//  $Log$
//  Revision 1.4  2002/05/03 11:23:06  gvd
//  Changed for new build environment
//
//  Revision 1.3  2002/03/01 08:29:35  gvd
//  Use new lofar_*.h for namespaces
//  Use Debug.h instead of firewall.h
//
//  Revision 1.2  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.1  2001/03/29 11:24:38  gvd
//  Added classes to write an MS
//
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <TFC_Storage/MSWriter.h>
#include <TFC_Storage/MSWriterImpl.h>
#ifdef HAVE_AIPSPP
# include <casa/Exceptions/Error.h>
#endif
#include <Common/lofar_iostream.h>

using namespace LOFAR;
using namespace casa;

MSWriter::MSWriter (const char* msName, double timeStep, int nantennas,
		    const float** antPos)
: itsWriter (0)
{
  // Form the antenna position vector.
  double* antPosOut = new double[3*nantennas];
  for (int i=0; i<nantennas; i++) {
    antPosOut[3*i + 0] = antPos[i][0];
    antPosOut[3*i + 1] = antPos[i][1];
    antPosOut[3*i + 2] = 0.;
  }
  try {
    itsWriter = new MSWriterImpl (msName, timeStep, nantennas, antPosOut);
    delete [] antPosOut;
#ifdef HAVE_AIPSPP
  } catch (AipsError x) {
    cerr << "MSWriter exception: " << x.getMesg() << endl;
    exit(0);
#endif
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

void MSWriter::write (int bandId, int fieldId, int timeCounter, int nrdata,
		      const fcomplex* data, const bool* flags)
{
  itsWriter->write (bandId, fieldId, timeCounter, nrdata,
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
