#ifndef VISAGENT_SRC_VISOUTPUTAGENT_H_HEADER_INCLUDED_BB8A6715
#define VISAGENT_SRC_VISOUTPUTAGENT_H_HEADER_INCLUDED_BB8A6715
    
#include <AppAgent/AppAgent.h>
#include <AppAgent/AppEventAgentBase.h>
#include <VisAgent/VisAgentVocabulary.h>
#include <VisCube/VisTile.h>
class AppEventSink;

namespace VisAgent
{

//##ModelId=3E00AA5100F9
class OutputAgent : public AppEventAgentBase
{
  public:
    //##ModelId=3E4143600221
    explicit OutputAgent (const HIID &initf = AidOutput)
      : AppEventAgentBase(initf) {}
    //##ModelId=3E41436101A6
    OutputAgent (AppEventSink &sink, const HIID &initf = AidOutput)
      : AppEventAgentBase(sink,initf) {}
    //##ModelId=3E50FAF103A1
    OutputAgent(AppEventSink *sink, int dmiflags, const HIID &initf = AidOutput)
      : AppEventAgentBase(sink,dmiflags,initf) {}

    
    //##ModelId=3E28276A0257
    //##Documentation
    //## Puts visibilities header onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end
    //##          CLOSED    stream closed
    virtual int putHeader   (const DataRecord::Ref::Xfer &hdr);

    // temporarily
    //##ModelId=3E28276D022A
    //##Documentation
    //## Puts next tile onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end
    //##          OUTOFSEQ  data is out of sequence (must send header first)
    //##          CLOSED    stream closed
    virtual int putNextTile (const VisTile::Ref::Xfer &tile);

    //##ModelId=3E41144B0245
    virtual string sdebug (int detail = 1, const string &prefix = "", const char *name = 0) const;
    
    
  private:
    //##ModelId=3E4235C203D4
    OutputAgent();


    //##ModelId=3E4235C301A5
    OutputAgent(const OutputAgent& right);

    //##ModelId=3E4235C302E6
    OutputAgent& operator=(const OutputAgent& right);

};

} // namespace VisAgent

#endif /* VISAGENT_SRC_VISOUTPUTAGENT_H_HEADER_INCLUDED_BB8A6715 */

