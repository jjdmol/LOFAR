#ifndef OCTOAGENT_SRC_OCTOMULTIPLEXER_H_HEADER_INCLUDED_B481AF8F
#define OCTOAGENT_SRC_OCTOMULTIPLEXER_H_HEADER_INCLUDED_B481AF8F
    
#include <OCTOPUSSY/Octoproxy.h>
#include <OCTOPUSSY/Message.h>
    
class OctoAgent;

//##ModelId=3E26BABA0069
class OctoMultiplexer
{
  public:
    //##ModelId=3E26BE240137
    OctoMultiplexer (AtomicID wpid);
  
    //##ModelId=3E26BE6701CD
    OctoMultiplexer (const OctoMultiplexer& right);
  
    //##ModelId=3E26BE670240
    virtual ~OctoMultiplexer ();
  
    //##ModelId=3E26BE670273
    OctoMultiplexer& operator=(const OctoMultiplexer& right);
  
    //##ModelId=3E26BE760036
    OctoMultiplexer&  addAgent (OctoAgent* agent);
    
    //##ModelId=3E26D2D6021D
    int   getEvent (HIID& id,ObjRef& data,const HIID& mask,int wait,int agent_id);
    //##ModelId=3E3FC3A601B0
    int   hasEvent (const HIID& mask,int agent_id);
    
    //##ModelId=3E26D91A02E3
    bool  subscribe (const HIID &id, int scope = Message::GLOBAL);
    //##ModelId=3E26D7CA01DA
    int   publish (MessageRef &mref, int scope = Message::GLOBAL);
    
    //##ModelId=3E26E30E01C5
    string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;
    
    //##ModelId=3E26E70701D5
    const char * debug ( int detail = 1,const string &prefix = "",
                         const char *name = 0 ) const
    { return Debug::staticBuffer(sdebug(detail,prefix,name)); }

  private:
      
    //##ModelId=3E3FC3A7000B
    int checkQueue (const HIID& mask,int wait,int agent_id);
      
    //##ModelId=3E26BD6B026E
    std::vector<OctoAgent*> agents;
    
    //##ModelId=3E3FC3A50362
    std::vector<bool> event_assignment;
    //##ModelId=3E3FC3A6004B
    std::vector<HIID> event_id;
    
    //##ModelId=3E26E56A0311
    int nassign;
    
    //##ModelId=3E3FC84E023C
    Message::Ref headmsg;
    
    //##ModelId=3E26BBC200FA
    Octoproxy::Identity proxy;

    
};



#endif /* OCTOAGENT_SRC_OCTOMULTIPLEXER_H_HEADER_INCLUDED_B481AF8F */
