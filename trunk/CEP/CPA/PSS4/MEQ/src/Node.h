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
#ifndef MEQSERVER_SRC_NODE_H_HEADER_INCLUDED_E5514413
#define MEQSERVER_SRC_NODE_H_HEADER_INCLUDED_E5514413
    
#include <DMI/DataRecord.h>
#include <MEQ/Result.h>
#include <MEQ/AID-MEQ.h>
#include <MEQ/TID-MEQ.h>
#include <map>
#include <vector>
    
#pragma aidgroup MEQ
#pragma aid Node Class Name State Child Children Result
#pragma types #MEQ::Node

namespace MEQ {

class Forest;
class Request;


//##ModelId=3F5F436202FE
class Node : public BlockableObject
{
  public:
    //##ModelId=3F5F43620304
    typedef CountedRef<Node> Ref;
  
  
    // these are flags returned by getResult(), indicating result properties
    //##ModelId=3F698825005B
    typedef enum {
      RES_WAIT    = 1,    // result not yet available, must wait
      RES_MUTABLE = 2,    // result may change for this request
      RES_UPDATED = 4,    // result updated since last request
    } ResultAttributes;

  
    //##ModelId=3F5F43E000A0
    Node();
    //##ModelId=3F5F44A401BC
    virtual ~Node();

    //##ModelId=3F5F45D202D5
    virtual void init (DataRecord::Ref::Xfer &initrec, Forest* frst);
    //##ModelId=3F83FAC80375
    void resolveChildren();
        
    //##ModelId=3F5F44820166
    const string & name() const;
    
    //##ModelId=3F5F441602D2
    const DataRecord & state() const;
    
    //##ModelId=3F5F445A00AC
    virtual void setState (const DataRecord &rec);
    
    //##ModelId=3F6726C4039D
    virtual int getResult (Result::Ref &resref, const Request&);
    
    
    int numChildren () const;
    
    Node & getChild (int i);
    
    Node & getChild (const HIID &id);

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
    { return TpMEQNode; }
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
    //##ModelId=3F83FADF011D
    virtual void checkChildren();
    //##ModelId=3F83F9A5022C
    DataRecord & wstate();
    
  private:
    //##ModelId=3F8433C20193
    void addChild (const HIID &id,Node &childnode );
      
    //##ModelId=3F5F4363030D
    DataRecord::Ref staterec_;
    //##ModelId=3F5F48040177
    string myname_;
    //##ModelId=3F5F43930004
    Forest *forest_;
    
    //##ModelId=3F8433C10295
    typedef std::map<HIID,int> ChildrenMap;
    //##ModelId=3F8433C102D3
    typedef std::pair<HIID,string> UnresolvedChild;
    //##ModelId=3F8433C10322
    typedef std::list<UnresolvedChild> UnresolvedChildren;
    
    //##ModelId=3F8433ED0337
    std::vector<Node::Ref> children_;
    //##ModelId=3F8433C2014B
    ChildrenMap child_map_;
    //##ModelId=3F8433C2016F
    UnresolvedChildren unresolved_children_;
};

inline int Node::numChildren () const
{
  return children_.size();
}

inline Node & Node::getChild (int i)
{
  return children_[i].dewr();
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

} // namespace MEQ

#endif /* MEQSERVER_SRC_NODE_H_HEADER_INCLUDED_E5514413 */
