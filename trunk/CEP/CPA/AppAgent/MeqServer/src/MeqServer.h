#ifndef MEQSERVER_SRC_MEQSERVER_H_HEADER_INCLUDED_D338579D
#define MEQSERVER_SRC_MEQSERVER_H_HEADER_INCLUDED_D338579D

#include <MEQ/Forest.h>
#include <AppUtils/VisPipe.h>
#include <MeqServer/VisDataMux.h>
#include <AppUtils/VisRepeater.h>

#pragma aidgroup MeqServer    
#pragma aid Node Name NodeIndex MeqServer
#pragma aid Create Delete Get Set State Request Resolve Child Children List
#pragma aid App Command Args Result Data Processing Error Message Code
#pragma aid Execute
    
namespace Meq
{

//##ModelId=3F5F195E013B
//##Documentation
//## Returns True if a valid node exists for the given index
class MeqServer : public VisRepeater
{
  public:
    //##ModelId=3F5F195E0140
    MeqServer();

    //##ModelId=3F608106021C
    virtual void run();

    //##ModelId=3F61920F01A8
    void createNode (DataRecord::Ref &out,DataRecord::Ref::Xfer &initrec);
    //##ModelId=3F61920F01FA
    void deleteNode (DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
    //##ModelId=3F61920F024E
    void getNodeState (DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
    //##ModelId=3F61920F02A4
    void setNodeState (DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
    //##ModelId=3F98D91A03B9
    void resolveChildren (DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
    //##ModelId=3F98D91B0064
    void getNodeList (DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
    
    void getNodeResult (DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
      
    //##ModelId=3F5F195E0156
    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

    //##ModelId=3F5F195E013F
    LocalDebugContext;

  private:
    //##ModelId=3F6196800325
    Node & resolveNode (const DataRecord &rec);
      
    //##ModelId=3F5F218F02BD
    Forest forest;
  
    //##ModelId=3F61920F0158
    typedef void (MeqServer::*PtrCommandMethod)(DataRecord::Ref &out,DataRecord::Ref::Xfer &in);
    //##ModelId=3F61920F016E
    typedef std::map<HIID,PtrCommandMethod> CommandMap;
    //##ModelId=3F61920F0193
    CommandMap command_map;
    //##ModelId=3F9CE0D3027D
    VisDataMux data_mux;


};

}; // namespace Meq

#endif /* MEQSERVER_SRC_MEQSERVER_H_HEADER_INCLUDED_D338579D */
