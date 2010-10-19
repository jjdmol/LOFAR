// MSWriter.h: interface class for filling a MeasurementSet for LofarSim
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


#ifndef TFLOPCORRELATOR_TFC_STORAGE_MSWRITER_H
#define TFLOPCORRELATOR_TFC_STORAGE_MSWRITER_H

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

//# Forward Declarations
class MSWriterImpl;

/// Class for filling a MeasurementSet
class MSWriter
{
public:
  // Construct the MS with a given name.
  // The timeStep is used by the write function
  // to calculate the time from the starting time and the timeCounter.
  // The antenna positions have to be given as xpos,ypos (in meters).
  // The WSRT is taken as the center of the array.
  // Thus antPos must have shape [2,nantennas].
  MSWriter (const char* msName, double startTime, double timeStep, uint nantennas,
	    const vector<double>& antPos);

  // Destructor
  ~MSWriter();

  // Add the definition of the next frequency band.
  // 1, 2 or 4 polarizations can be given.
  // 1 is always XX; 2 is XX,YY; 4 is XX,XY,YX,YY.
  // The frequencies have to be given in Hz.
  // The first version assumes that all channels are adjacent and that the
  // the reference frequency is the center of the entire band. From that
  // it calculates the center frequency for each channel.
  // The second version can be used to specify the center frequency and width
  // for each channel. So both arrays must have nchannels entries.
  // Note that the total bandwidth is calculated from the minimum and
  // maximum channel frequency. Thus it is not the sum of all widths.
  // <br>It returns the id (0-relative seqnr) of the band.
  // <group>
  int addBand (int npolarizations, int nchannels,
	       double refFreq, double chanWidth);
  int addBand (int npolarizations, int nchannels,
	       double refFreq, const double* chanFreqs,
	       const double* chanWidths);
  // </group>

  // Add the definition of the next field (i.e. beam).
  // The angles have to be given in radians.
  // <br>It returns the id (0-relative seqnr) of the field.
  int addField (double azimuth, double elevation);

  // Write a data array for the given band and field.
  // The data array is a 4D complex array with axes polarization, channel,
  // antenna2, antenna1 (polarization is the most rapidly varying axis).
  // Only the part where antenna2>antenna1 is used.
  // The flag array has the same shape as the data array. Flag==True
  // means that the corresponding data point is flagged as invalid.
  // The flag array is optional. If not given, all flags are False.
  // All data will be written with sigma=0 and weight=1.
  // <br>The number of data points (nrdata) given should match the
  // number of antennas, bands, and polarizations for this bandId.
  void write (int& rowNr, int bandId, int fieldId, int channelId, 
	      int timeCounter, int nrdata,
	      const fcomplex* data, const bool* flags = 0);

  // Get the number of antennas.
  int nrAntennas() const;

  // Get the number of bands.
  int nrBands() const;

  // Get the number of fields.
  int nrFields() const;

  // Get the number of different polarization setups.
  int nrPolarizations() const;

  // Get the number of exposures.
  int nrTimes() const;


private:
  // Forbid copy constructor and assignment by making them private.
  // <group>
  MSWriter (const MSWriter&);
  MSWriter& operator= (const MSWriter&);
  // </group>


  //# Define the data.
  //# Note we use pointers to an MSWriterImpl class, so MSWriter
  //# does not need any AIPS++ include file.
  MSWriterImpl* itsWriter;
};

} // namespace LOFAR

#endif
