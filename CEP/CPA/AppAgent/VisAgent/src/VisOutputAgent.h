#ifndef VISAGENT_SRC_VISOUTPUTAGENT_H_HEADER_INCLUDED_BB8A6715
#define VISAGENT_SRC_VISOUTPUTAGENT_H_HEADER_INCLUDED_BB8A6715
    
#include <AppAgent/AppAgent.h>
#include <VisCube/VisTile.h>

//##ModelId=3E00AA5100F9
class VisOutputAgent : virtual public AppAgent
{
  public:
    //##ModelId=3E28276A0257
    //##Documentation
    //## Puts visibilities header onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end
    //##          CLOSED    stream closed
    virtual int putHeader   (DataRecord::Ref &hdr) = 0;

    // temporarily
    //##ModelId=3E28276D022A
    //##Documentation
    //## Puts next tile onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end    //## Gets next available tile from input stream. If wait=True, blocks until
    //##          CLOSED    stream closed
    virtual int putNextTile (VisTile::Ref &tile) = 0;

  protected:
    //##ModelId=3E3AA73B002A
      AppAgent::getEvent;
    
};



#endif /* VISAGENT_SRC_VISOUTPUTAGENT_H_HEADER_INCLUDED_BB8A6715 */
