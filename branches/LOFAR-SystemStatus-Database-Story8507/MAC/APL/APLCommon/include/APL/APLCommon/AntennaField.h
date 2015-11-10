//#  AntennaField.h: Class to manage the antenna subsets.
//#
//#  Copyright (C) 2010
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: AntennaField.h 15145 2010-03-05 10:35:46Z overeem $

#ifndef LOFAR_APPLCOMMON_ANTENNAFIELD_H
#define LOFAR_APPLCOMMON_ANTENNAFIELD_H

// \file AntennaField.h
// Class to manage the antenna subsets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ApplCommon/AntField.h>
#include <blitz/array.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {

// \addtogroup package
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// This class represents an antenna array. The LOFAR remote station will initially
// have two anntena arrays. One for the low-band antennas, and one for the high-band
// antennas. Alternative configurations of the antennas are described in AntennaSets.
class AntennaField
{
public:
	explicit AntennaField (const string&	filename);
	virtual ~AntennaField();

	// all about the bits
	// [antNr, pol, xyz]
	const blitz::Array<double, 3>& AntPos(const string& fieldName) const
		{ return (itsAntPos[name2Index(fieldName)]); }

	// [rcu, xyz]
	const blitz::Array<double, 2>& RCUPos(const string& fieldName) const
		{ return (itsRCUPos[name2Index(fieldName)]); }

	// [xyz]
	const blitz::Array<double, 1>& Centre(const string& fieldName) const
		{ return (itsFieldCentres[name2Index(fieldName)]); }

	// [rcu]
	const blitz::Array<double, 1>& RCULengths(const string& fieldName) const
		{ return (itsRCULengths[name2Index(fieldName)]); }

	// [xyz]
	const blitz::Array<double, 1>& normVector(const string& fieldName) const
		{ return (itsNormVectors[name2Index(fieldName)]); }

	// [3, 3]
	const blitz::Array<double, 2>& rotationMatrix(const string& fieldName) const
		{ return (itsRotationMatrix[name2Index(fieldName)]); }

	int nrAnts(const string& fieldName) const
		{ return (itsAntPos[name2Index(fieldName)].extent(blitz::firstDim)); }

	bool isAntennaField(const string&	name) const
		{ return (name2Index(name) >= 0); }

	int ringNr(const string& fieldName) const
		{ return ((fieldName == "HBA1") ? 1 : 0); }

private:
	// Copying is not allowed
	AntennaField();
	AntennaField(const AntennaField&	that);
	AntennaField& operator=(const AntennaField& that);

	// translate name of antennaField to index in blitzArrays
	int name2Index(const string& fieldName) const
          { return itsAntField.name2Index (fieldName); }

        // Create Blitz arrays referencing the AFArray data.
        void makeArray1d (AntField::AFArray& array,
                          blitz::Array<double,1>& out);
        void makeArray2d (AntField::AFArray& array,
                          blitz::Array<double,2>& out);
        void makeArray3d (AntField::AFArray& array,
                          blitz::Array<double,3>& out);
        void makeRCUPos (AntField::AFArray& array,
                         blitz::Array<double,2>& out);


	//# --- Datamembers ---
        AntField itsAntField;

	// Note: we use a vector<blitz::Array> so that every blitz array can have its own sizes.
        // The blitz arrays reference the vectors in the AntField object.
	vector<blitz::Array<double,1> >		itsFieldCentres;	// [ (x,y,z) ]

	vector<blitz::Array<double,2> >		itsRCUPos;			// [ rcuNr, (x,y,z) ]

	vector<blitz::Array<double,3> >		itsAntPos;			// [ antNr, pol, (x,y,z) ]

	vector<blitz::Array<double,1> >		itsNormVectors;		// [ (x,y,z) ]

	vector<blitz::Array<double,2> >		itsRotationMatrix;	// [ 3, 3 ]

	// during calculations we often need the length of the vectors.
	vector<blitz::Array<double,1> >		itsRCULengths;		// [ len ]
};

// Make one instance of the AntennaField globally accessable.
AntennaField* 	globalAntennaField();


// @}
} // namespace LOFAR

#endif
