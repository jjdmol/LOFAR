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

#include <uvplot/UVPDataAtom.h>

#if(DEBUG_MODE)
#include <cassert>
#endif



//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom()
  : itsData(0),
    itsFlags(0)
{
}





//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom(unsigned int               numberOfChannels,
                         const UVPDataAtomHeader &  header)
  : itsHeader(header),
    itsData(numberOfChannels),
    itsFlags(numberOfChannels)
{
}





//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom(const UVPDataAtom & other)
  :itsData(other.itsData.size()),
   itsFlags(other.itsFlags.size())
{
  itsHeader = other.itsHeader;
  setData(&(other.itsData.front()));
  setFlags( other.itsFlags );
}





//====================>>>  UVPDataAtom::setChannels  <<<====================

void UVPDataAtom::setChannels(unsigned int numberOfChannels)
{
  itsData  = std::vector<ComplexType>(numberOfChannels);
  itsFlags = std::vector<bool>(numberOfChannels);
}





//====================>>>  UVPDataAtom::setHeader  <<<====================

void UVPDataAtom::setHeader(const UVPDataAtomHeader& header)
{
  itsHeader = header;
}






//====================>>>  UVPDataAtom::setData  <<<====================

void UVPDataAtom::setData(unsigned int       channel,
                          const ComplexType& data)
{
  itsData[channel] = data;
}






//====================>>>  UVPDataAtom::setData  <<<====================

void UVPDataAtom::setData(const std::vector<ComplexType>& data)
{
#if(DEBUG_MODE)
  assert(data.size() == itsData.size());
#endif

  itsData = data;
}







//====================>>>  UVPDataAtom::setData  <<<====================

void UVPDataAtom::setData(const ComplexType* data)
{
  ComplexType*       i   = &(itsData.front());

  memcpy(i, data, itsData.size()*sizeof(ComplexType));
}







//====================>>>  UVPDataAtom::setData  <<<====================

void UVPDataAtom::setData(const LoVec_fcomplex& data)
{
#if(DEBUG_MODE)
  assert(data.shape()[0] == int(itsData.size()));
#endif
  ComplexType*       i   = &(itsData.front());
  for(LoVec_fcomplex::const_iterator iter = data.begin();
      iter != data.end();
      iter++){
    *i++ = *iter;
  }
    
}







//====================>>>  UVPDataAtom::setFlags  <<<====================

void UVPDataAtom::setFlags(const std::vector<bool>& flags)
{
  itsFlags = flags;
}









//====================>>>  UVPDataAtom::setFlags  <<<====================

void UVPDataAtom::setFlags(const LoVec_int& flags)
{
  std::vector<bool>::iterator fiter = itsFlags.begin();
  for(LoVec_int::const_iterator iter = flags.begin();
      iter != flags.end();
      iter++){
    *fiter++ = *iter;
  }

}








//====================>>>  UVPDataAtom::setFlag  <<<====================

void UVPDataAtom::setFlag(unsigned int channel,
                          bool         flag)
{
  itsFlags[channel] = flag;
}







//=================>>>  UVPDataAtom::getNumberOfChannels  <<<=================

unsigned int  UVPDataAtom::getNumberOfChannels() const
{
  return itsData.size();
}





//====================>>>  UVPDataAtom::getData  <<<====================

const UVPDataAtom::ComplexType * UVPDataAtom::getData(unsigned int channel) const
{
#if(DEBUG_MODE)
  assert(channel < itsData.size());
#endif
  return &(itsData[channel]);
}




//====================>>>  UVPDataAtom::getFlag  <<<====================

const bool UVPDataAtom::getFlag(unsigned int channel) const
{
#if(DEBUG_MODE)
  assert(channel < itsFlags.size());
#endif
  return itsFlags[channel];
}





//==================>>>  UVPDataAtom::getFlagBegin  <<<===================

UVPDataAtom::FlagIterator UVPDataAtom::getFlagBegin() const
{
  return itsFlags.begin();
}




//==================>>>  UVPDataAtom::getFlagEnd  <<<===================

UVPDataAtom::FlagIterator UVPDataAtom::getFlagEnd() const
{
  return itsFlags.end();
}




//====================>>>  UVPDataAtom::getHeader <<<====================

const UVPDataAtomHeader& UVPDataAtom::getHeader() const
{
  return itsHeader;
}





//====================>>>  UVPDataAtom::store  <<<====================

void UVPDataAtom::store(std::ostream& out) const
{
  unsigned int N = itsData.size();
  itsHeader.store(out);
  out.write((const char*)&N, sizeof(unsigned int));
  out.write((const char*)&(itsData.front()), N*sizeof(ComplexType));
  FlagIterator end = itsFlags.end();
  for(FlagIterator i = itsFlags.begin(); i != end; i++) {
    unsigned char ch(*i);
    out.write((const char*)&ch, 1);
  }
}







//====================>>>  UVPDataAtom::load  <<<====================

void UVPDataAtom::load(std::istream& in)
{
  unsigned int N(0);

  itsHeader.load(in);
  in.read((char*)&N, sizeof(unsigned int));
  
  itsData  = std::vector<ComplexType>(N);
  itsFlags = std::vector<bool>(N);
  
  in.read((char*)&(itsData.front()), N*sizeof(ComplexType));
  std::vector<bool>::iterator end = itsFlags.end();
  for(std::vector<bool>::iterator i = itsFlags.begin(); i != end; i++) {
    unsigned char ch(0);
    in.read((char*)&ch, 1);
    *i=bool(ch);
  }
}
