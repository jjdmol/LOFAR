//#  Node.h: base MeqNode class
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
#ifndef MeqSERVER_SRC_NODE_H_HEADER_INCLUDED_E5514413
#define MeqSERVER_SRC_NODE_H_HEADER_INCLUDED_E5514413
    
#include <DMI/DataRecord.h>
#include <MEQ/Result.h>
#include <MEQ/AID-Meq.h>
#include <MEQ/TID-Meq.h>
#include <map>
#include <vector>
    
#pragma aidgroup Meq
#pragma types #Meq::Node

namespace Meq {

class Forest;
class Request;

//##ModelId=3F5F436202FE
class Node : public BlockableObject
{
  public:
    //##ModelId=3F5F43620304
    typedef CountedRef<Node> Ref;
  
    //##ModelId=3F698825005B
    // these are flags returned by execute(), indicating result properties
    typedef enum {
      RES_WAIT          = 0x01,    // result not yet available, must wait
      RES_UPDATED       = 0x02,    // result updated since last request
          
      RES_FAIL          = 0x80,    // result is complete fail
          
      // bitmask of result dependencies
      RES_DEPEND_MASK  = 0xFFFF00, // full dependency mask
      RES_DEPEND_NBITS = 16,       // number of bits in dependency mask
      RES_DEPEND_LSB   = 0x100,    // LSB of dependency mask
          
      // predefine some constants for application-specific dependencies
      RES_DEP_VOLATILE = 0x100,    // result is volatile (depends on external events)
      RES_DEP_DOMAIN   = 0x200,    // depends on domain
      RES_DEP_CONFIG   = 0x400,    // depends on config
      RES_DEP_VALUE    = 0x800,    // depends on parm values
          
    } ResultAttributes;
    
    // for generic dependency treatment:
    // returns bit indicating dependency on request ID index #n
    static int RES_DEP (int n)
    { return RES_DEPEND_LSB<<n; }
    
  
    //##ModelId=3F5F43E000A0
    // Child labels may be specified in constructror as a C array of HIIDs.
    // If nchildren>=0, specifies that an exact number of children is expected.
    // If nchildren<0, spefifies that at least -(nchildren+1) are expected
    // (hence -1 implies no checking)
    Node (int nchildren=-1,const HIID *labels = 0);
        
    //##ModelId=3F5F44A401BC
    virtual ~Node();

    //##ModelId=3F5F45D202D5
    virtual void init (DataRecord::Ref::Xfer &initrec, Forest* frst);
    //##ModelId=3F83FAC80375
    void resolveChildren();
        
    //##ModelId=3F5F44820166
    const string & name() const;
    
    int nodeIndex() const 
    { return node_index_; }
    
    string className() const
    { return objectType().toString(); }
    
    //##ModelId=3F5F441602D2
    const DataRecord & state() const;
    
    void setNodeIndex (int nodeindex);
    
    //##ModelId=3F5F445A00AC
    void setState (DataRecord &rec);
    
    //##ModelId=3F6726C4039D
    int execute (Result::Ref &resref, const Request &);

    const HIID & currentRequestId ();
    
    //##ModelId=3F85710E002E
    int numChildren () const;
    
    //##ModelId=3F85710E011F
    Node & getChild (int i);
    
    //##ModelId=3F85710E028E
    Node & getChild (const HIID &id);
    
    //##ModelId=3F98D9D20201
    int getChildNumber (const HIID &id);
    
    //##ModelId=3F98D9D20372
    Forest & forest ();

    // implement abstract methods inherited from BlockableObject 
    //##ModelId=3F5F4363030F
    //##Documentation
    //## Clones a node. 
    //## Currently not implemented (throws exception)
    virtual CountedRefTarget* clone(int flags = 0, int depth = 0) const;
    //##ModelId=3F5F43630313
    //##Documentation
    //## Returns the class TypeId
    virtual TypeId objectType() const
    { return TpMeqNode; }
    //##ModelId=3F5F43630315
    //##Documentation
    //## Un-serialize.
    //## Currently not implemented (throws exception)
    virtual int fromBlock(BlockSet& set);
    //##ModelId=3F5F43630318
    //##Documentation
    //## Serialize.
    //## Currently not implemented (throws exception)
    virtual int toBlock(BlockSet &set) const;
    
    //##ModelId=3F5F48180303
    //##Documentation
    //## Standard debug info method
    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

    //##ModelId=3F8433C1039E
    LocalDebugContext;
    
  protected:
    // ----------------- virtual methods defining node behaviour --------------
      
    //##ModelId=3F83FADF011D
    // called from resolveChildren(), meant to check children types if the node
    // requires specific children. Throw exception on failure. 
    virtual void checkChildren()
    {} // base version does no checking
    
    //##ModelId=3F98D9D2006B
    // called from init(), meant to check the initrec for required fields,
    // and to fill in any missing defaults. Throws exception on failure 
    // (i.e. if a required field is missing)
    virtual void checkInitState (DataRecord &rec);
    
    // called from init() and setState(), meant to update internal state
    // in accordance to rec. If initializing==true (i.e. when called from 
    // init()), rec is a complete state record.
    virtual void setStateImpl (DataRecord &rec,bool initializing);
    
    // called from execute() when a new request is received. Should return
    // true if it is OK to proceed, false otherwise (RES_WAIT will be returned
    // by execute() on a false). Nodes tied to external data sources may need 
    // to override this.
    virtual bool readyForRequest (const Request &)
    { return true; } // base version always ready
        
    // called from execute() to process the request rider, if any.
    virtual void processRider (const DataRecord &)
    {} // base version does nothing
    
    // Called from execute() to collect the child results for a given request.
    // Child_results vector is pre-sized to the number of children.
    // The method is expected to pass the request on to the children,  
    // collect their results in the vector, and return the accumulated
    // result code. If RES_FAIL is returned, then resref should point
    // to a Result with the fails in it; this result will be returned by
    // execute() immediately with the RES_FAIl code.
    // Default version does just that. If any child returns RES_FAIL,
    // collects all fails into resref and returns RES_FAIL. If no children
    // fail, resref is left untouched.
    // Nodes should only reimplement this if they prefer to poll children 
    // themselves (i.e. the Solver). 
    virtual int pollChildren (std::vector<Result::Ref> &child_results,
                              Result::Ref &resref,
                              const Request &req);

    //##ModelId=3F98D9D100B9
    // Called from execute() to compute the result of a request.
    //  childres is a vector of child results (empty if no children);
    //  req is request; newreq is true if the request is new.
    // Result should be created and attached to resref. Return code indicates
    // result properties. If the RES_WAIT flag isis returned, then no result is 
    // expected; otherwise a Result must be attached to the ref.
    // RES_FAIL may be returned to indicate complete failure.
    virtual int getResult (Result::Ref &resref, 
                           const std::vector<Result::Ref> &childres,
                           const Request &req,bool newreq);
    
    // ----------------- misc helper methods ----------------------------------
    //##ModelId=3F83F9A5022C
    // write-access to the state record
    DataRecord & wstate();
    
    // Checks for cached result; if hit, attaches it to ref and returns true.
    // On a miss, clears the cache (NB: for now!)
    bool getCachedResult (int &retcode,Result::Ref &ref,const Request &req);
    
    // Conditionally stores result in cache according to current policy.
    // Returns the retcode.
    int cacheResult (const Result::Ref &ref,int retcode);
    
    // Clears cache (not recursively)
    void clearLocalCache ();
      
    // creates a message of the form "Node CLASS ('NAME'): MESSAGE"
    string makeMessage (const string &msg) const
      { return  "Node " + className() + "('" + name() + "'): " + msg; }
    
    // These exception are meant to be thrown from methods like init(),
    // getResult(), processRider() and setStateImpl() when something goes 
    // wrong. The type of the exception indicates whether any cleanup is 
    // required.
    EXCEPTION_CLASS(FailWithCleanup,LOFAR::Exception)
    EXCEPTION_CLASS(FailWithoutCleanup,LOFAR::Exception)
  
    //  NodeThrow can be used to throw an exception, with the message
    // passed through makeMessage()
    #define NodeThrow(exc,msg) \
      { THROW(exc,makeMessage(msg)); }
    #define NodeThrow1(msg) \
      { THROW(FailWithoutCleanup,makeMessage(msg)); }

    // Helper method for init(). Checks that initrec field exists, throws a
    // FailWithoutCleanup with the appropriate message if it doesn't.
    // Defined as macro so that exception gets proper file/line info
    #define requiresInitField(rec,field) \
      { if( !(rec)[field].exists() ) \
         NodeThrow(FailWithoutCleanup,"missing initrec."+(field).toString()); } \
    // Helper method for init(). Checks that initrec field exists and inserts
    // a default value if it doesn't.
    #define defaultInitField(rec,field,deflt) \
      { if( !(rec)[field].exists() ) (rec)[field] = (deflt); }
        
    // Helper method for setStateImpl(). Meant to check for immutable state 
    // fields. Checks if record rec contains the given field, throws a 
    // FailWithoutCleanup with the appropriate message if it does.
    // Defined as macro so that exception gets proper file/line info
    #define protectStateField(rec,field) \
      { if( (rec)[field].exists() ) \
          NodeThrow(FailWithoutCleanup,"state."+(field).toString()+" not reconfigurable"); }
          
    // Helper method for setStateImpl(). Checks if rec[field] exists, if yes,
    // assigns it to 'out', returns true. Otherwise returns false.
    template<class T>
    bool getStateField (T &out,const DataRecord &rec,const HIID &field)
    { if( rec[field].exists() ) {
        out = rec[field].as(Type2Type<T>());
        return true;
      } else {
        return false;
      }
    }
      
  private:
    //##ModelId=3F9505E50010
    void processChildSpec (NestableContainer &children,const HIID &id);
    //##ModelId=3F8433C20193
    void addChild (const HIID &id,Node *childnode);
    
    void setCurrentRequest (const Request &req);

    const HIID * child_labels_;      
    int check_nchildren_;
    
    //##ModelId=3F5F4363030D
    DataRecord::Ref staterec_;
    
    //##ModelId=3F5F48040177
    string myname_;
    int node_index_;
    //##ModelId=3F5F43930004
    Forest *forest_;
    
    HIID current_reqid_;
    
    // cached result of current request
    Result::Ref cache_result_;
    int cache_retcode_;
    
    //##ModelId=3F8433C10295
    typedef std::map<HIID,int> ChildrenMap;
    //##ModelId=3F8433C10322
    typedef std::list<string> UnresolvedChildren;
    
    //##ModelId=3F8433ED0337
    std::vector<Node::Ref> children_;
    //##ModelId=3F8433C2014B
    ChildrenMap child_map_;
    //##ModelId=3F8433C2016F
    UnresolvedChildren unresolved_children_;
    
    std::vector<HIID> config_groups_;
};

inline const HIID & Node::currentRequestId ()
{
  return current_reqid_;
}

//##ModelId=3F85710E002E
inline int Node::numChildren () const
{
  return children_.size();
}

//##ModelId=3F5F441602D2
inline const DataRecord & Node::state() const
{
  return *staterec_;
}

//##ModelId=3F83F9A5022C
inline DataRecord & Node::wstate() 
{
  return staterec_();
}

//##ModelId=3F5F44820166
inline const string & Node::name() const
{
  return myname_;
}

//##ModelId=3F98D9D20372
inline Forest & Node::forest() 
{
  return *forest_;
}

} // namespace Meq

#endif /* MeqSERVER_SRC_NODE_H_HEADER_INCLUDED_E5514413 */
