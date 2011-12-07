#ifndef GPA_CONVERTER_H
#define GPA_CONVERTER_H

#include <GCF/PAL/GCF_PVSSPort.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {  
class GCFPVBlob;
  }
  namespace TM
  {
class GCFEvent;
  }
  namespace PAL 
  {

class GPAConverter : public GCFPVSSUIMConverter
{
  public:
    bool uimMsgToGCFEvent(unsigned char* buf, unsigned int length, Common::GCFPVBlob& gcfEvent);
    bool gcfEventToUIMMsg(Common::GCFPVBlob& gcfEvent, Common::GCFPVBlob& uimMsg);
    
  private:  
    ssize_t recv (void* buf, size_t count);
    void encodeEvent(TM::GCFEvent& e, list<string>& msg);
    const unsigned char* _msgBuffer;
    unsigned int _bytesLeft;
};

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif 
