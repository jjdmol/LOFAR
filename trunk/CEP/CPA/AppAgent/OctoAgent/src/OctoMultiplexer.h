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
    void  addAgent (OctoAgent* agent);
    //##ModelId=3E26D2D6021D
    int   pollEvents (bool wait);
    //##ModelId=3E26D2D602E5
    void  claimEvent ();
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
    //##ModelId=3E26BBC200FA
    Octoproxy::Identity proxy;
    
    //##ModelId=3E26D2D60013
    typedef std::vector<OctoAgent*> AgentList;
    //##ModelId=3E26BD6B026E
    AgentList agents;
    
    //##ModelId=3E26E56A0311
    int nevents;
    
};



#endif /* OCTOAGENT_SRC_OCTOMULTIPLEXER_H_HEADER_INCLUDED_B481AF8F */
