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

#ifndef GSM_ABSTRACTSOURCE_H
#define GSM_ABSTRACTSOURCE_H

// $Id$

#include <string>
#include <vector>
#include <iostream>


#include <MNS/MeqExpr.h>
#include <MNS/MeqParm.h>


class Table;
class MDirection;

//! The Global Sky Model
namespace GSM
{


const unsigned int NUMBER_OF_POLARIZATIONS = 4;

//! The enum SourceType is used as an alternative for RTTI.
enum SourceType 
{
  NONE  = 0,
  POINT = 1,
  GAUSS,
  SHAPELET,
  WAVELET,
  PIXON,
  IMAGE,
  COMPOUND   = 10,
  OTHER_BASE = 100
};


//! The Stokes parameters.
enum Stokes
{
  I=0,
  Q,
  U,
  V
};



//! Base class for the sky model source classes.
/*! itsSourceType must be set in the constructor of all derived classes.
 */
class AbstractSource
{
public:
  
  //! Constructor
  /*!
    \param type indicates the source type, e.g. POINT, GAUSS, etc...
    \param catNumber is the catalog number of the source.
    \param name is a canonical source name, like "M31", if available.
   */
           AbstractSource(SourceType         type,
                          unsigned int       catNumber = 0,
                          const std::string& name      = "");

  //! Destructor
  virtual ~AbstractSource();

  //! Returns the source type.
  SourceType  getType() const;
  
  //! \returns catalog number of the source
  unsigned int getCatalogNumber() const;
  
  //! \returns source name, if available
  std::string  getName() const;


  //! Source position is stored in J2000 coordinates.
  /*! For synthesis imaging one needs APPARENT coordinates. These can
      be asked to MDirection though.
   */
  virtual MDirection  getPosition(double time,
                                  double frequency,
                                  Stokes stokes) const;

  //! Get MeqExpr objects for ra and dec
  /*!
     \param ra  Right Ascension in J2000 coordinates.
     \param dec Declination in J2000 coordinates.
   */
  virtual void        getPositionExpressions(MeqExpr* ra,
                                             MeqExpr* dec);

  //! Returns expressions that give total flux.
  /*! \returns pointers to expressions as a function of time and
    frequency for the total flux in stokes I,Q,U,V respectively.

    \param expressions is assumed to be of size() NUMBER_OF_POLARIZATIONS.
   */
  virtual void  getFluxExpressions(std::vector<MeqExpr *> &expressions) = 0;
  


  //! Stores its contents in an Aips++ table at a specified row
  /*!
   In the first version, column names are:
     
     * NUMBER    ScalarColumn<int>
     * NAME      ScalarColumn<std::string>
     * TYPE      ScalarColumn<int>
     * RAPARMS   ArrayColumn<double>
     * DECPARMS  ArrayColumn<double>
     * IPARMS    ArrayColumn<double>
     * QPARMS    ArrayColumn<double>
     * UPARMS    ArrayColumn<double>
     * VPARMS    ArrayColumn<double>

     
   */
  virtual void load(Table&       table,
                    unsigned int row);

  //! Loads state from an Aips++ table.
  /*! Description of the table
      format can be found in the description of \method load.
  */
  virtual void store(Table&       table,
                     unsigned int row);




  //! \returns the total number of parameters.
  virtual unsigned int getNumberOfParameters() const;

  //! Get pointers to all MeqParm objects. Including the position.
  /*! first param always is RA, second is dec. Then the rest of the
    parameters follows.
  */
  virtual unsigned int  getParameters(std::vector<MeqParm* > &parameters);
  
  //! Makes deep copy of parameters.
  /*! First is RA, second is Dec, then the other ones.
   */
  virtual unsigned int setParameters(const std::vector<MeqParm* > &parameters);

  


  //! Writes state in machine readable ASCII. May be read back by readAscii.
  virtual std::ostream&  writeAscii(std::ostream& out) const;
  
  virtual std::istream&  readAscii(std::istream& in);

  //! Writes state as human readable ASCII. Not readable by readAscii.
  virtual std::ostream&  writeNiceAscii(std::ostream& out)const;

protected:
private:
  
  unsigned int itsCatalogNumber;
  std::string  itsName;
  SourceType   itsSourceType;

  MeqParm*     itsRA;
  MeqParm*     itsDec;

};


}
#endif // GSM_ABSTRACTSOURCE_H
