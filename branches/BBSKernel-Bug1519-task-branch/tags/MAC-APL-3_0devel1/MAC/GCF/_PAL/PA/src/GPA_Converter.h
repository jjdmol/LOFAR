#ifndef GPA_CONVERTER_H
#define GPA_CONVERTER_H

#include <GCF/PAL/GCF_PVSSPort.h>
#include <Common/lofar_list.h>


class GCFEvent;
class GCFPVBlob;


class GPAConverter : public GCFPVSSUIMConverter
{
  public:
    bool uimMsgToGCFEvent(unsigned char* buf, unsigned int length, GCFPVBlob& gcfEvent);
    bool gcfEventToUIMMsg(GCFPVBlob& gcfEvent, GCFPVBlob& uimMsg);
    
  private:  
    ssize_t recv (void* buf, size_t count);
    void encodeEvent(GCFEvent& e, list<string>& msg);
    const unsigned char* _msgBuffer;
    unsigned int _bytesLeft;
};


#endif 
