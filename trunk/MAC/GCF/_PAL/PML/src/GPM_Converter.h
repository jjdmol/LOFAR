#ifndef GPM_CONVERTER_H
#define GPM_CONVERTER_H

#include <GCF/PAL/GCF_PVSSPort.h>
#include <Common/lofar_list.h>


class GCFEvent;
class GCFPVBlob;


class GPMConverter : public GCFPVSSUIMConverter
{
  public:
    bool uimMsgToGCFEvent(unsigned char* buf, unsigned int length, GCFPVBlob& gcfEvent);
    bool gcfEventToUIMMsg(GCFPVBlob& gcfEvent, GCFPVBlob& uimMsg);
    
  private:  
};


#endif 
