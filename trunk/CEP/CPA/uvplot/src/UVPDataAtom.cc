// Copyright notice

// $ID

#include <UVPDataAtom.h>


#if(DEBUG_MODE)
#include <cassert>
#endif



//====================>>>  UVPDataAtom::UVPDataAtom  <<<====================

UVPDataAtom::UVPDataAtom(int    numberOfChannels,
                         double time)
  : itsData(numberOfChannels),
    itsTime(time)    
{
}





//====================>>>  UVPDataAtom::setChannels  <<<====================

void UVPDataAtom::setChannels(unsigned int numberOfChannels)
{
  itsData = std::vector<double_complex>(numberOfChannels);
}





//====================>>>  UVPDataAtom::setData  <<<====================

void UVPDataAtom::setData(unsigned int channel,
                          const double_complex& data)
{
  itsData[channel] = data;
}






//====================>>>  UVPDataAtom::setData  <<<====================

void UVPDataAtom::setData(const std::vector<double_complex>& data)
{
#if(DEBUG_MODE)
  assert(data.size() == itsData.size());
#endif

  itsData = data;
}





//=================>>>  UVPDataAtom::getNumberOfChannels  <<<=================

unsigned int  UVPDataAtom::getNumberOfChannels() const
{
  return itsData.size();
}





//====================>>>  UVPDataAtom::getData  <<<====================

const double_complex * UVPDataAtom::getData(unsigned int channel) const
{
#if(DEBUG_MODE)
  assert(channel < itsData.size());
#endif
  return &(itsData[channel]);
}
