#include "MeqServer.h"
#include "AID-MeqServer.h"
#include "DMI/AID-DMI.h"
    
using Debug::ssprintf;
using namespace AppControlAgentVocabulary;
    
namespace MEQ 
{
  
InitDebugContext(MeqServer,"MeqServer");
  
//##ModelId=3F5F195E0140
MeqServer::MeqServer()
{
  command_map["Create.Node"] = &MeqServer::createNode;
  command_map["Delete.Node"] = &MeqServer::deleteNode;
  command_map["Get.Node.State"] = &MeqServer::getNodeState;
  command_map["Set.Node.State"] = &MeqServer::setNodeState;
}


//##ModelId=3F6196800325
Node & MeqServer::resolveNode (const DataRecord &rec)
{
  int nodeindex = rec[AidNodeIndex].as<int>(-1);
  if( nodeindex>0 )
    return noderep.get(nodeindex);
  string name = rec[AidName].as<string>("");
  FailWhen( !name.length(),"either nodeindex or name must be specified");
  cdebug(3)<<"looking up node name "<<name<<endl;
  return noderep.findNode(name);
}


void MeqServer::createNode (DataRecord::Ref &out,DataRecord::Ref::Xfer &initrec)
{
  cdebug(2)<<"creating node ";
  cdebug(3)<<initrec->sdebug(3);
  cdebug(2)<<endl;
  int nodeindex = noderep.create(initrec);
  out()[AidNodeIndex] = nodeindex;
  out()[AidMessage] = ssprintf("node %d created",nodeindex);
}

void MeqServer::deleteNode (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  int nodeindex = (*in)[AidNodeIndex].as<int>(-1);
  if( nodeindex<0 )
  {
    string name = (*in)[AidName].as<string>("");
    cdebug(3)<<"looking up node name "<<name<<endl;
    FailWhen( !name.length(),"either nodeindex or name must be specified");
    nodeindex = noderep.findIndex(name);
    FailWhen( nodeindex<0,"node '"+name+"' not found");
  }
  cdebug(2)<<"deleting node "<<nodeindex<<endl;
  string name = noderep.get(nodeindex).name();
  noderep.remove(nodeindex);
  out()[AidMessage] = ssprintf("node %d (%s) deleted",nodeindex,name.c_str());
}

void MeqServer::getNodeState (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  Node & node = resolveNode(*in);
  cdebug(3)<<"getState for node "<<node.name()<<" ";
  cdebug(4)<<in->sdebug(3);
  cdebug(3)<<endl;
  out.attach(node.state(),DMI::READONLY|DMI::ANON);
}

void MeqServer::setNodeState (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  Node & node = resolveNode(*in);
  cdebug(3)<<"setState for node "<<node.name()<<endl;
  node.setState(*in);
  out.attach(node.state(),DMI::READONLY|DMI::ANON);
}

//##ModelId=3F608106021C
void MeqServer::run ()
{
  verifySetup(True);
  DataRecord::Ref initrec;
  // keep running as long as start() on the control agent succeeds
  while( control().start(initrec) == AppState::RUNNING )
  {
    HIID vdsid;
    control().setStatus(AidA,"none");
    control().setStatus(AidB,1);
    control().setStatus(AidC,2.0);
    // run main loop
    while( control().state() > 0 )  // while in a running state
    {
      // check for commands from the control agent
      HIID cmdid;
      DataRecord::Ref cmddata;
      if( control().getCommand(cmdid,cmddata,AppEvent::WAIT) == AppEvent::SUCCESS 
          && cmdid.matches(AppCommandMask) )
      {
        // strip off the App.Control.Command prefix -- the -1 is not very
        // nice because it assumes a wildcard is the last thing in the mask.
        // Which it usually will be
        cmdid = cmdid.subId(AppCommandMask.length()-1);
        cdebug(3)<<"received app command "<<cmdid.toString()<<endl;
        int request_id = 0;
        string error_str;
        DataRecord::Ref retval(DMI::ANONWR);
        try
        {
          cmddata.privatize(DMI::WRITE);
          request_id = (*cmddata)[FRequestId].as<int>(0);
          ObjRef ref = cmddata()[AidArgs].remove();
          DataRecord::Ref args;
          if( ref.valid() )
          {
            FailWhen(!ref->objectType()==TpDataRecord,"invalid args field");
            args = ref.ref_cast<DataRecord>();
          }
          CommandMap::const_iterator iter = command_map.find(cmdid);
          if( iter != command_map.end() )
          {
            // execute the command, catching any errors
            (this->*(iter->second))(retval,args);
          }
          else // command not found
            error_str = "unknown command "+cmdid.toString();
        }
        catch( std::exception &exc )
        {
          error_str = exc.what();
        }
        catch( ... )
        {
          error_str = "unknown exception while processing command";
        }
        // in case of error, insert error message into return value
        if( error_str.length() )
          retval()[AidError] = error_str;
        HIID reply_id = CommandResultPrefix|cmdid;
        if( request_id )
          reply_id |= request_id;
        control().postEvent(reply_id,retval);
      }
      // .. but ignore them since we only watch for state changes anyway
    }
    // go back up for another start() call
  }
  cdebug(1)<<"exiting with control state "<<control().stateString()<<endl;
  control().close();
}

//##ModelId=3F5F195E0156
string MeqServer::sdebug(int detail, const string &prefix, const char *name) const
{
  return "MeqServer";
}

};
