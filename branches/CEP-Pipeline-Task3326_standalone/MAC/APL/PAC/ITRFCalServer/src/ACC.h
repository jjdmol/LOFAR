//#  -*- mode: c++ -*-
//#  ACC.h: definition of the Auto Correlation Cube class
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
//#  $Id: ACC.h 6967 2005-10-31 16:28:09Z wierenga $

#ifndef ACC_H_
#define ACC_H_

#include <blitz/array.h>
#include <Common/lofar_complex.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/LBA_Calibration/lba_calibration.h>		// the matlab stuff

namespace LOFAR {
  namespace ICAL {

class ACC
{
public:
	// constructors
	ACC(int nsubbands, int nantennas);
	~ACC();

	// Set the antenna selection mask to use in getACM. Dimensions (nantennas (all) x 2 (pol))
	// @param selection 2-dimensional blitz array with selection to apply
	// @note precondition: antenna_selection.extent(firstDim) == itsACC.extent(fourthDim)
	void setSelection(const blitz::Array<bool, 2>& antenna_selection);

	// Get a subband and single polarized slice of the ACC cube.
	// @param subband Subband for which to return the ACM.
	// @param pol1 0 == x, 1 == y
	// @param pol2 0 == x, 1 == y
	// @param timestamp The timestamp of the specified subband is returned.
	// If an invalid subband is specified Timestamp(0,0) is returned.
	// @return The ACM for the specified subband and polarizations is returned.
	blitz::Array<std::complex<double>, 2> getACM(int subband, int pol1, int pol2, RTC::Timestamp& timestamp);

	// Update an ACM.
	void updateACM(int 											subband, 
				   const RTC::Timestamp& 						timestamp,
		 		   const blitz::Array<std::complex<double>, 4>&	newacm);

	// Get the size of the array.
	int getSize() const 		{ return (itsAntCount*itsSubbandCount*sizeof(std::complex<double>)); }	 // x 2 ?????

	// Get number of subbands.
	// @return the number of subbands in this ACC
	int getNSubbands() const	{ return (itsSubbandCount); }

	// Get number of antennas
	// @return the number of antennas in this ACC
	int getNAntennas() const	{ return (itsAntCount); }

	// Get a reference to the ACC array.
	// @return The 5-dimensional ACC array.
	mwArray*	xdata() 	 { return(itsXACC); }
	mwArray*	ydata() 	 { return(itsYACC); }
	mwArray*	timestamps() { return(itsTimestamps); }

	// Is the array valid? If not don't use it.
	void needsStart() 	{	itsNeedStart = true; itsIsAborted = false; }
	void started() 	  	{	itsNeedStart = false; itsIsReady = false; }
	bool isWaiting4Start() const  {	return (itsNeedStart); }

	// Is the array valid? If not don't use it.
	bool isAborted() const	{	return (itsIsAborted); }
	void abort()			{	itsIsAborted = true; }

	// Is the array valid? If not don't use it.
	bool isReady() const {	return (itsIsReady); }
	void setReady(bool	newValue) {	itsIsReady = newValue; }

	// Get the ACC from file. The ACC in the file needs
	// to have the shape that is already defined.
	// @param filename Name of the file to read.
	// @return true on success
	bool getFromFile(string filename);

	// Get ACC from binary file assuming the itsACC dimensions.
	bool getFromBinaryFile(string filename);

	// call for operator<<
	ostream& print (ostream& os) const;

private:
	// prevent default construction and editing
	ACC();
	ACC(const ACC&	that);
	ACC& operator=(const ACC& that);

	// ACC is a five dimensional array of complex numbers with dimensions
	// [ nantennas x nantennas x nsubbands ]
	mwArray*		itsXACC;		// xpol ACC info
	mwArray*		itsYACC;		// ypol ACC info

	// m_time is a 1 dimensional array with a timestamp for each subband.
	mwArray* 		itsTimestamps;	// [ subbands ]

	// process trigger flags
	bool 			itsNeedStart; 		// needs total refresh of the data
	bool 			itsIsReady; 		// finished with the data.
	bool			itsIsAborted;		// abort usage.

	int				itsAntCount;     	// number of selected antennas
	int				itsSubbandCount;	// max nr subbands.

	blitz::Array<bool, 2>				itsAntennaSelection; // selection of antennas to be used by ::getACM
	blitz::Array<std::complex<double>, 2>	itsCurrentACM;       // the ACM last returned by ::getACM
};

// operator<<
inline ostream& operator<<(ostream& os, const ACC& acc)
{
	return (acc.print(os));
}

  }; // namespace ICAL
}; // namespace LOFAR

#endif /* ACC_H_ */

