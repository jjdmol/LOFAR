// Copyright notice

#include <uvplot/UVPDataAtom.h>

#if(DEBUG_MODE)
#include <cassert>
#endif



//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom()
  : itsData(0)
{
}





//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom(unsigned int               numberOfChannels,
                         const UVPDataAtomHeader &  header)
  : itsHeader(header),
    itsData(numberOfChannels)
{
}





//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom(const UVPDataAtom & other)
  :itsData(other.itsData.size())
{
  itsHeader = other.itsHeader;
  setData(&(other.itsData.front()));
}





//====================>>>  UVPDataAtom::setChannels  <<<====================

void UVPDataAtom::setChannels(unsigned int numberOfChannels)
{
  itsData = std::vector<ComplexType>(numberOfChannels);
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




//====================>>>  UVPDataAtom::getHeader <<<====================

const UVPDataAtomHeader& UVPDataAtom::getHeader() const
{
  return itsHeader;
}
