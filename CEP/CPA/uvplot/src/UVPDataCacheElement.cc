// Copyright notice



#include <UVPDataCacheElement.h>

//=============>>>  UVPDataCacheElement::UVPDataCacheElement  <<<=============

UVPDataCacheElement::UVPDataCacheElement()
  : itsDirty(false)
{
}







//==================>>>  UVPDataCacheElement::isDirty  <<<==================

bool UVPDataCacheElement::isDirty() const
{
  return itsDirty();
}





//==================>>>  UVPDataCacheElement::setDirty  <<<==================

void UVPDataCacheElement::setDirty(bool dirty)
{
  itsDirty = dirty;
}
