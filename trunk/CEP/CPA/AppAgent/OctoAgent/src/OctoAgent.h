#ifndef OCTOAGENT_SRC_OCTOAGENT_H_HEADER_INCLUDED_833D2E92
#define OCTOAGENT_SRC_OCTOAGENT_H_HEADER_INCLUDED_833D2E92

#include <DMI/DataRecord.h>
#include <AppAgent/AppAgent.h>
#include <OCTOPUSSY/WPInterface.h>
#include <OctoAgent/AID-OctoAgent.h>
#include <map>
    
class OctoMultiplexer;
    
#pragma aidgroup OctoAgent
#pragma aid Agent Map Receive Post
#pragma aid Default Scope Unmapped Prefix Local Host Global
#pragma aid Priority Lowest Lower Low Normal High Higher
    
namespace OctoAgentVocabulary 
{
  const HIID
      FDefaultMapField  = AidAgent | AidMap,
      FMapReceive       = AidReceive, 
      FMapPost          = AidPost, 
      
      FDefaultScope    = AidDefault | AidScope,
      FDefaultPriority = AidDefault | AidPriority,
      FUnmappedPrefix  = AidUnmapped | AidPrefix;
};

//##ModelId=3DF9FECD014B
class OctoAgent : virtual public AppAgent
{
  private:
    // dummy default argument for functions below
    //##ModelId=3E26D477006C
    static DataRecord _dum;
  
  public:
    //##ModelId=3E394D4D0279
    LocalDebugContext;  
      
    //##ModelId=3E26DA36005C
    OctoAgent (const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);
    //##ModelId=3E091DDB02F2
    OctoAgent (OctoMultiplexer &pxy,const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);
    //##ModelId=3E26DA370170
    virtual ~OctoAgent();

    //##ModelId=3E091DDD02B8
    void attach (OctoMultiplexer &pxy,const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);
    
    //##ModelId=3E27F30B019B
    //##Documentation
    //## Agent initialization method. This sets the receive and post maps
    //## from data[map_field_name][FMapReceive] and
    //##      data[map_field_name][FMapPost]
    virtual bool init(const DataRecord::Ref &data);

    //##ModelId=3E0A295102A5
    //##Documentation
    //## Sets up a mapping of input events to message IDs
    void setReceiveMap (const DataRecord &map);

    //##ModelId=3E0A296A02C8
    //##Documentation
    //## Sets up a mapping of output events to message IDs
    void setPostMap (const DataRecord &map);

    //##ModelId=3E26CBBC03E4
    bool mapReceiveEvent  (HIID &out, const HIID &in) const;
    
    //##ModelId=3E2FEAD10188
    bool mapPostEvent     (HIID &out, const HIID &in) const;
    
    //##ModelId=3E0918BF0299
    //##Documentation
    //## Gets next event from proxy's message queue. 
    virtual int getEvent (HIID &id, DataRecord::Ref &data, const HIID &mask, bool wait = False);
    //##ModelId=3E096F2103B3
    //##Documentation
    //## Gets next event from proxy's message queue. This version will return
    //## any type of payload, not just DataRecords, hence the ObjRef data
    //## argument.
    virtual int getEvent (HIID &id, ObjRef &data, const HIID &mask, bool wait = False);
    
    //##ModelId=3E0918BF02F0
    //##Documentation
    //## Checks for event in proxy WP's message queue.
    virtual int hasEvent (const HIID &mask = HIID(), bool outOfSeq = False);

    //##ModelId=3E0918BF034D
    //##Documentation
    //## Publishes an event as a message on behalf of the proxy
    virtual void postEvent (const HIID &id, const DataRecord::Ref &data = DataRecord::Ref());
    //##ModelId=3E2FD67D0246
    //##Documentation
    //## Publishes an event as a message on behalf of the proxy. This
    //## version will publish any type of payload
    virtual void postEvent(const HIID &id, const ObjRef &data);
    
    //##ModelId=3E26D4A80155
    void cacheEvent (const HIID &id, const ObjRef &data);
    
    //##ModelId=3E26E2F901E3
    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;
    
  protected:
    // helper functions for parsing the event maps. These look up default
    // arguments in the maps
    //##ModelId=3E0A34E7020E
    int getDefaultScope    (const DataRecord &map);
    //##ModelId=3E0A4C7B0006
    int getDefaultPriority (const DataRecord &map);
    
    // checks ID for scope or priority prefix, strips it off and interprets
    // as integer value. If no valid prefix is found, returns input value.
    //##ModelId=3E0A34E801D6
    int resolveScope       (HIID &id,int scope);
    //##ModelId=3E0A4C7D007A
    int resolvePriority    (HIID &id,int priority);

  private:
    //##ModelId=3E26DA3602F4
    OctoAgent(const OctoAgent& right);
    //##ModelId=3E26DA370287
    OctoAgent& operator=(const OctoAgent& right);
    
    //##ModelId=3E0A34E60005
    typedef struct { HIID id; int scope,priority; } EventMapEntry;
    //##ModelId=3E0A34E600D8
    typedef std::map<HIID,EventMapEntry> EventMap;
    //##ModelId=3E0A3D2501EE
    typedef EventMap::const_iterator EMCI;
    
    //##ModelId=3E0A3578009A
    //##Documentation
    //## Maps message IDs to input events
    EventMap receive_map;
    //##ModelId=3E0A359F0050
    //## Maps posted events to message IDs
    EventMap post_map;

    //##ModelId=3E0A29D202D1
    //##Documentation
    //## True if events without a mapping are to be published anyway 
    //## (as unmapped_prefix.event_id, with unmapped_scope and _priority)
    bool publish_unmapped_events;
    //##ModelId=3E0A367402F0
    HIID unmapped_prefix;
    //##ModelId=3E0A3A980187
    int unmapped_scope;
    //##ModelId=3E0A4C7A0008
    int unmapped_priority;
    
    //##ModelId=3E26CA060164
    bool pending_event;
    //##ModelId=3E26CA0601E1
    ObjRef pending_event_data;
    //##ModelId=3E26D53202A3
    HIID pending_event_id;
    
    //##ModelId=3E26BD6B029C
    OctoMultiplexer *proxy;
    
    //##ModelId=3E27F5E502C0
    HIID map_field_name;
};

#endif /* OCTOAGENT_SRC_OCTOAGENT_H_HEADER_INCLUDED_833D2E92 */
