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

#if !defined(UVPDATAHEADER_H)
#define UVPDATAHEADER_H

// $Id$



#include <iostream>
#include <string>

//! Stores auxiliary data when a chunk is received through OCTOPUSSY
/*! All indices are zero-relative*/
class UVPDataHeader
{
 public:

  UVPDataHeader(int                  correlation       = 0,
                int                  numberOfBaselines = 0,
                int                  numberOfTimeslots = 0,
                int                  numberOfChannels  = 0,
                int                  fieldID           = 0,
                const std::string&  fieldName         = "" );

  int        itsCorrelation;
  int        itsNumberOfBaselines;
  int        itsNumberOfTimeslots;
  int        itsNumberOfChannels;
  int        itsFieldID;
  std::string itsFieldName;
  

 protected:
 private:
};


std::ostream &operator <<(std::ostream &out, const UVPDataHeader &header);



#endif //UVPDATAHEADER_H
