#ifndef Gateways_h
#define Gateways_h 1
    
#include "DMI/HIID.h"
#include "PSCF/AID-PSCF.h"

// this includes declarations common to all gateways
    
// Use checksum in gateway transmissions? This increases CPU usage
// during local transfers by a factor of 2
#define GATEWAY_CHECKSUM 0
    
// Re-advertise servers. If set, all open GWServers will publish
// a GW.Server.Bound message at regular intervals. This should re-enable
// any connections that have dropped out. Quite a paranoid feature
#define ADVERTISE_SERVERS 0

#pragma aid ConnectionMgrWP GWServerWP GWClientWP GatewayWP 
#pragma aid GW Client Server Bind Error Fatal Bound Remote Up Down Network Type
#pragma aid Duplicate Host Port Peers Connected Connection Add Network Local Open
    
// gateway-related messages
const HIID 
  // Messages published by server
  // common prefix
  MsgGWServer(AidGW|AidServer),      
  // This is used to advertise when a server socket has been bound...
  MsgGWServerOpen(AidGW|AidServer|AidOpen),                     
  // ...for local unix sockets
  // Payload: [AidHost] = hostname (string), [AidPort] = port.
  MsgGWServerOpenLocal(MsgGWServerOpen|AidLocal),           
  // ...for network tcp sockets. Payload is the same, where
  // "host" is the base path, and port is a socket number (i.e. the
  // actual socket is available as host:port)
  MsgGWServerOpenNetwork(MsgGWServerOpen|AidNetwork),
  // common prefix for error messages
  MsgGWServerError(MsgGWServer|AidError),           
  // bind() failed. Payload as above
  MsgGWServerBindError(MsgGWServerError|AidBind),
  // other (fatal) error. Payload as above, plus [AidEerror] = error string
  MsgGWServerFatalError(MsgGWServerError|AidFatal),

  // Messages generated by GatewayWPs
  // Common prefix
  MsgGWRemote(AidGW|AidRemote),
  // Error: duplicate connection to remote peer. Payload: [AidHost], [AidPort],
  // if this was a client connection
  MsgGWRemoteDuplicate(MsgGWRemote|AidDuplicate),
  // Connected to remote peer (GW.Remote.Up.process.host)
  MsgGWRemoteUp(MsgGWRemote|AidUp),
  // Disconnected from remote peer (GW.Remote.Down.process.host)
  MsgGWRemoteDown(MsgGWRemote|AidDown),

  // Local data fieldnames
  // list of connections
  GWPeerList(AidGW|AidPeers),
  // local server port (-1 when no server yet)
  GWNetworkServer(AidGW|AidNetwork|AidPort),
  GWLocalServer(AidGW|AidLocal|AidPort),

// dummy const  
  GWNull();
  
// opens standard set of client/server gateways
class Dispatcher;
void initGateways (Dispatcher &dsp);
  
#endif
