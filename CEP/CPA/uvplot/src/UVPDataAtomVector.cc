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

#include <uvplot/UVPDataAtomVector.h>


//===============>>>  UVPDataAtomVector::UVPDataAtomVector  <<<===============

UVPDataAtomVector::UVPDataAtomVector()
  : std::vector<const UVPDataAtom *>(),
    itsMinRe(0),
    itsMaxRe(0),
    itsMinIm(0),
    itsMaxIm(0),
    itsMin(0),
    itsMax(0)
{
}




//===============>>>  UVPDataAtomVector::~UVPDataAtomVector  <<<===============

UVPDataAtomVector::~UVPDataAtomVector()
{
}



//===============>>>  UVPDataAtomVector::minRe  <<<===============

double UVPDataAtomVector::minRe() const
{
  return itsMinRe;
}





//===============>>>  UVPDataAtomVector::maxRe  <<<===============

double UVPDataAtomVector::maxRe() const
{
  return itsMaxRe;
}




//===============>>>  UVPDataAtomVector::minIm  <<<===============

double UVPDataAtomVector::minIm() const
{
  return itsMinIm;
}






//===============>>>  UVPDataAtomVector::maxIm  <<<===============

double UVPDataAtomVector::maxIm() const
{
  return itsMaxIm;
}



//===============>>>  UVPDataAtomVector::min  <<<===============

double UVPDataAtomVector::min() const
{
  return itsMin;
}




//===============>>>  UVPDataAtomVector::max  <<<===============

double UVPDataAtomVector::max() const
{
  return itsMax;
}





//===============>>>  UVPDataAtomVector::add  <<<===============

void UVPDataAtomVector::add(const UVPDataAtom *atom,
                            bool               honourFlags)
{
  push_back(atom);

  double MinRe(0);
  double MaxRe(0);
  double MinIm(0);
  double MaxIm(0);

  const UVPDataAtom::ComplexType* value = atom->getData(0);
  unsigned int                    N     = atom->getNumberOfChannels();
  const UVPDataAtom::ComplexType* end   = value + N;
  UVPDataAtom::FlagIterator       flag  = atom->getFlagBegin();

  if(N > 0) {
    if( !(*flag && honourFlags)) {
      MinRe = (*value).real();
      MaxRe = MinRe;
      MinIm = (*value).imag();
      MaxIm = MinIm;
    }
    value++;
    flag++;

    while(value < end) {
      double real = (*value).real();
      double imag = (*value).imag();
      value++;

      if( !(*flag && honourFlags)) {
        MinRe = ( real < MinRe ? real : MinRe);
        MaxRe = ( real > MaxRe ? real : MaxRe);
        MinIm = ( imag < MinIm ? imag : MinIm);
        MaxIm = ( imag > MaxIm ? imag : MaxIm);
      }
      flag++;
    }
    
    if(size() == 1) {
      itsMinRe = MinRe;
      itsMaxRe = MaxRe;
      itsMinIm = MinIm;
      itsMaxIm = MaxIm;
    } else {
      if(MinRe < itsMinRe) {
        itsMinRe = MinRe;
      }
      if(MaxRe > itsMaxRe) {
        itsMaxRe = MaxRe;
      }
      if(MinIm < itsMinIm) {
        itsMinIm = MinIm;
      }
      if(MaxIm > itsMaxIm) {
        itsMaxIm = MaxIm;
      }
    } // If size() ...

    itsMin = (itsMinIm < itsMinRe ? itsMinIm : itsMinRe);
    itsMax = (itsMaxIm > itsMaxRe ? itsMaxIm : itsMaxRe);
    
  } // If N > 0
  
}


