

#include <DH_DFTRequest.h>
#include <DH_DFTResult.h>
#include <Transport/TH_Socket.h>

namespace LOFAR { 

class DFTServer_ClientStub
{
 public:
  DFTServer_ClientStub() {
    const ParameterSet myPS("params.ps");
    itsTHProtoRequest = new TH_Socket(myPS.getString("DFTConnection.ClientHost"),   // sendhost
				 myPS.getString("DFTConnection.ServerHost"),   // recvhost
				 myPS.getInt("DFTConnection.RequestPort")    // port
                        );    
    itsTHProtoResult = new TH_Socket(myPS.getString("DFTConnection.ServerHost"),   // sendhost
				  myPS.getString("DFTConnection.ClientHost"),   // recvhost
				  myPS.getInt("DFTConnection.ResultPort")    // port
                        );    
  };
  ~DFTServer_ClientStub() {};

  DH_DFTRequest      itsRequestDH;   //todo: need to set ID...
  DH_DFTResult       itsResultDH;  //todo: need to set ID...

  TH_Socket *itsTHProtoRequest;
  TH_Socket *itsTHProtoResult;
  bool      itsSynchronisity;
};

} //namespace

