//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(UVPDATAATOMHEADER_H)
#define UVPDATAATOMHEADER_H

// $Id$

#include <vector>
#include <iostream>

#include <values.h>

//! Simple class with public(!) members for managing data regarding the
//! location in the measurement set of UVPDataAtom objects.
/*!  Its primary purpose is to make looking up and storing UVPDataAtom
     objects faster by minimizing the number of parameters in function
     calls.  Pass objects of this class as a reference.
*/

class UVPDataAtomHeader
{
 public:

  enum Correlation{None=0,
                   I=1,
                   Q=2,
                   U=3,
                   V=4,
                   RR=5,
                   RL=6,
                   LR=7,
                   LL=8,
                   XX=9,
                   XY=10,
                   YX=11,
                   YY=12};
  
  enum DataType{Raw = 0,
                Corrected,
                Model};

  //! Zero based antenna indices.
  unsigned int itsAntenna1;
  unsigned int itsAntenna2;

  //! Time of mid-integration in seconds.
  double       itsTime;

  //! Exposure time in seconds.
  float        itsExposureTime;

  //! Polarization type of correlation.
  Correlation  itsCorrelationType;

  //! Zero based field indentifier.
  unsigned int itsFieldID;
  
  //! The U,V,W coordinates in meters
  double       itsUVW[3];

  //! What kind of data is this anyway?
  /*! Possible kinds are Raw, Corrected and Model.
   */
  DataType     itsDataType;



  //! Constructor
  /*!
    class sortHeader to force the right sort order.
   */
  UVPDataAtomHeader(unsigned int antenna1        = 0,
                    unsigned int antenna2        = 0,
                    double       time            = -MAXDOUBLE,
                    float        exposureTime    = 0.0,
                    Correlation  correlationType = None,
                    unsigned int fieldID         = 0,
                    const std::vector<double>& uvw = std::vector<double>(3,0),
                    DataType     dataType        = Raw);
  
  //! Less-than operator. Keys are
  //itsAntenna1-itsAntenna2-itsCorrelationType-itsTime.
  bool operator < (const UVPDataAtomHeader &other) const;
  
  //! Stores internal state in binary format.
  /*!
    - Bytes  0- 3:     unsigned int     Antenna 1
    - Bytes  4- 7:     unsigned int     Antenna 2
    - Bytes  8-15:     double           Time (mid integration)
    - Bytes 16-19:     float            Exposure time
    - Bytes 20-23:     unsigned int     Field ID
    - Bytes 24-31:     double           U (ITRF, meters)
    - Bytes 32-39:     double           V (ITRF, meters)
    - Bytes 40-48:     double           W (ITRF, meters)
    - Byte  49   :     unsigned char    Correlation type
    - Byte  50   :     unsigned char    Data type
   */
  void store(std::ostream &out) const;

  //! Loads internal state in binary format.
  void load(std::istream &in);

};


#endif // UVPDATAATOMHEADER_H
