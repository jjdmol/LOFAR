// Copyright notice

#if !defined(UVPDATAATOMHEADER_H)
#define UVPDATAATOMHEADER_H

// $Id$

#include <vector>

#include <values.h>

//! Simple class with public(!) members for managing data regarding the
//! location in the measurement set of UVPDataAtom objects.
/*!  Its primary purpose is to make looking up and storing UVPDataAtom
     objects faster by minimizing the number of parameters in function
     calls.  Pass objects of this class as a reference.
     
     Always call "sortAntennae" when you have changed either
     itsANtenna1 or itsAntenna2. It will make sure that itsAntenna1 is
     ALWAYS less than or equal to itsAntenna2.
 */

class UVPDataAtomHeader
{
 public:

  enum Correlation{XX=1,
                   XY=2,
                   YX=3,
                   YY=4,
                   RR=5,
                   RL=6,
                   LR=7,
                   LL=8};
  
  enum DataType{Raw,
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
                    Correlation  correlationType = XX,
                    unsigned int fieldID         = 0,
                    const std::vector<double>& uvw = std::vector<double>(3,0),
                    DataType     dataType        = Raw);

  //! Less-than operator. Keys are itsAntenna1-itsAntenna2-itsCorrelationType-itsTime.
  bool operator < (const UVPDataAtomHeader &other) const;

  //! Makes sure that itsAntenan1 <= itsAntenna2
  /*! This method should always be called when itsAntenna1 or
    itsAntenna2 is changed
   */
  void sortAntennae();

};


#endif // UVPDATAATOMHEADER_H
