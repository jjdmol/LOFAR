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

#if !defined(UVPDATAATOMVECTOR_H)
#define UVPDATAATOMVECTOR_H

// $Id$

#include <uvplot/UVPDataAtom.h>

#include <vector>


//! A list of UVPDataAtom objects.
/*!It maintains min/max values for both the real and imaginary parts
  of the visibilities. Use the "add" method instead of the "push_back"
  method. A UVPDataAtomVector does not destruct the objects it points
  to. It is supposed to live shorter than the UVPDataAtom objects that
  it points to. */

class UVPDataAtomVector:public std::vector<const UVPDataAtom *>
{
public:
   UVPDataAtomVector();
  ~UVPDataAtomVector();
  
  double minRe() const;
  double maxRe() const;
  double minIm() const;
  double maxIm() const;

  double min() const;
  double max() const;

  //!If honourFlags == true, ignore flagged data in min/max computation
  void   add(const UVPDataAtom *atom,
             bool               honourFlags=false);
  
protected:
private:
  
  double itsMinRe;
  double itsMaxRe;
  double itsMinIm;
  double itsMaxIm;
  
  double itsMin;
  double itsMax;
};


#endif
