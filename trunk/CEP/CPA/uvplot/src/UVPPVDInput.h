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

#if !defined(UVPPVDINPUT_H)
#define UVPPVDINPUT_H

// $Id$

#include <string>

#include <DMI/BOIO.h>

#include <uvplot/UVPDataAtom.h>
#include <uvplot/UVPDataSet.h>

//! Interface to PVD dataset
/*! Uses BOIO to read the data from a file */

class UVPPVDInput
{
public:
  //! Constructor
  UVPPVDInput(const std::string &filename);
  
  unsigned int numberOfAntennae() const;

  //  unsigned int numberOfPolarizations() const;

  unsigned int numberOfChannels() const;

  bool getDataAtoms(UVPDataSet* dataset,
                    unsigned int ant1,
                    unsigned int ant2);
  
protected:
private:

  unsigned int itsNumberOfAntennae;
  unsigned int itsNumberOfBaselines;
  unsigned int itsNumberOfPolarizations;
  unsigned int itsNumberOfChannels;
  UVPDataAtom  itsDataAtom;
  BOIO         itsBOIO;

};

#endif // UVPPVDInput
