// Copyright notice

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







//====================>>>  UVPDataAtom::setFlags  <<<====================

void UVPDataAtom::setFlags(const std::vector<bool>& flags)
{
  itsFlags = flags;
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
  out.write((const void*)&N, sizeof(unsigned int));
  out.write((const void*)&(itsData.front()), N*sizeof(ComplexType));
  FlagIterator end = itsFlags.end();
  for(FlagIterator i = itsFlags.begin(); i != end; i++) {
    unsigned char ch(*i);
    out.write(&ch, 1);
  }
}







//====================>>>  UVPDataAtom::load  <<<====================

void UVPDataAtom::load(std::istream& in)
{
  unsigned int N(0);

  itsHeader.load(in);
  in.read((void*)&N, sizeof(unsigned int));
  
  itsData  = std::vector<ComplexType>(N);
  itsFlags = std::vector<bool>(N);
  
  in.read((void*)&(itsData.front()), N*sizeof(ComplexType));
  std::vector<bool>::iterator end = itsFlags.end();
  for(std::vector<bool>::iterator i = itsFlags.begin(); i != end; i++) {
    unsigned char ch(0);
    in.read(&ch, 1);
    *i=bool(ch);
  }
}
