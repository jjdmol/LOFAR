// MSWriterImpl.h: implementation for filling a MeasurementSet
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
//  Revision 1.5  2002/05/03 11:23:06  gvd
//  Changed for new build environment
//
//  Revision 1.4  2002/03/01 08:29:35  gvd
//  Use new lofar_*.h for namespaces
//  Use Debug.h instead of firewall.h
//
//  Revision 1.3  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/04/04 14:31:12  gvd
//  Write better UVW coordinates; improved writing in general
//
//  Revision 1.1  2001/03/29 11:24:38  gvd
//  Added classes to write an MS
//
//////////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_TFC_STORAGE_MSWRITERIMPL_H
#define TFLOPCORRELATOR_TFC_STORAGE_MSWRITERIMPL_H

//# Includes
#include <Common/LofarTypes.h>
#ifdef HAVE_AIPSPP
#include <casa/aips.h>
#endif

//# Forward Declarations
#ifdef HAVE_AIPSPP
namespace casa
{
class MPosition;
class MeasFrame;
class MeasurementSet;
class MSMainColumns;
template<class T> class Block;
template<class T> class Vector;
template<class T> class Cube;
}
#endif

namespace LOFAR
{

// Class for filling a MeasurementSet.
// It is a wrapper class to hide the AIPS++ intricacies from
// the LOFAR environment which uses STL.

class MSWriterImpl
{
public:
  // Construct the MS with a given name.
  // The time of construction is used as the starting time of
  // the observation. The timeStep (in sec) is used by the write function
  // to calculate the time from the starting time and the timeCounter.
  // The antenna positions have to be given in meters (x,y,z)
  // relative to the center (which is set to Westerbork). So antPos
  // must have shape [3,nantennas].
  MSWriterImpl (const char* msName, double timeStep,
		int nantennas, const double* antPos);

  // Destructor
  ~MSWriterImpl();

  // Add the definition of the next frequency band.
  // 1, 2 or 4 polarizations can be given.
  // 1 is always XX; 2 is XX,YY; 4 is XX,XY,YX,YY.
  // The frequencies have to be given in Hz.
  // <group>
  int addBand (int npolarizations, int nchannels,
	       double refFreq, double chanWidth);
  int addBand (int npolarizations, int nchannels,
	       double refFreq, const double* chanFreqs,
	       const double* chanWidths);
  // </group>

  // Add the definition of the next field (i.e. beam).
  // The angles have to be given in degrees.
  int addField (double azimuth, double elevation);

  // Write a data array for the given band and field.
  // The data array is a 4D complex array with axes polarization, channel,
  // antenna2, antenna1. Only the part where antenna2>antenna1 is used.
  // The flag array has the same shape as the data array. Flag==True
  // means the the corresponding data point is flagged as invalid.
  // The flag array is optional. If not given, all flags are False.
  // All data will be written with sigma=0 and weight=1.
  void write (int bandId, int fieldId, int timeCounter, int nrdata,
	      const fcomplex* data, const bool* flags);

  // Get the number of antennas.
  int nrAntennas() const
    { return itsNrAnt; }

  // Get the number of bands.
  int nrBands() const
    { return itsNrBand; }

  // Get the number of fields.
  int nrFields() const
    { return itsNrField; }

  // Get the number of different polarization setups.
  int nrPolarizations() const;

  // Get the number of exposures.
  int nrTimes() const
    { return itsNrTimes; }

private:
  // Forbid copy constructor and assignment by making them private.
  // <group>
  MSWriterImpl (const MSWriterImpl&);
  MSWriterImpl& operator= (const MSWriterImpl&);
  // </group>

#ifdef HAVE_AIPSPP
  // Create the MS and fill its subtables as much as possible.
  void createMS (const char* msName, 
		 const casa::Block<casa::MPosition>& antPos);

  // Add a band.
  int addBand (int npolarizations, int nchannels,
	       double refFreq, const casa::Vector<double>& chanFreqs,
	       const casa::Vector<double>& chanWidths);

  // Add a polarization to the subtable.
  // Return the row number where it is added.
  int addPolarization (int npolarizations);

  // Fill the various subtables (at the end).
  // <group>
  void fillAntenna (const casa::Block<casa::MPosition>& antPos);
  void fillFeed();
  void fillObservation();
  void fillProcessor();
  void fillState();
  // </group>

  // Update the times in various subtables at the end of the observation.
  void updateTimes();
#endif

  //# Define the data.
  int itsNrBand;                     //# nr of bands
  int itsNrField;                    //# nr of fields (beams)
  int itsNrAnt;                      //# nr of antennas (stations)
  int itsNrTimes;                    //# nr of exposures
#ifdef HAVE_AIPSPP
  double itsTimeStep;                //# duration of each exposure (sec)
  double itsStartTime;               //# start time of observation (sec)
  casa::Block<casa::Int>* itsNrPol;  //# nr of polarizations for each band
  casa::Block<casa::Int>* itsNrChan; //# nr of channels for each band
  casa::Block<casa::Int>* itsPolnr;  //# rownr in POL subtable for each band
  casa::Cube<casa::Double>* itsBaselines;      //# XYZ (in m) of each baseline
  double itsArrayLon;                //# longitude of array center
  casa::MPosition*      itsArrayPos; //# Position of array center
  casa::MeasFrame*      itsFrame;    //# Frame to convert to apparent coordinates
  casa::MeasurementSet* itsMS;
  casa::MSMainColumns*  itsMSCol;
#endif
};

} // namespace LOFAR

#endif
