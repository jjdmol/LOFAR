
#include <ACC/ParameterSet.h>
#include <DH_DFTRequest.h>
#include <DH_DFTResult.h>
#include <Transport/TH_Socket.h>

namespace LOFAR { 

class DFTServer_ServerStub
{
public:
  DFTServer_ServerStub() {
    const ParameterSet myPS("DFTServer.param");
    itsTHProtoRequest = new TH_Socket(myPS.getString("DFTConnection.ClientHost"),   // sendhost
				      myPS.getString("DFTConnection.ServerHost"),   // recvhost
				      myPS.getInt("DFTConnection.RequestPort")    // port
				      );
    itsTHProtoResult = new TH_Socket(myPS.getString("DFTConnection.ServerHost"),   // sendhost
				     myPS.getString("DFTConnection.ClientHost"),   // recvhost
				     myPS.getInt("DFTConnection.ResultPort")    // port
				     );    
  };
  ~DFTServer_ServerStub() {};
  
  DH_DFTRequest      itsRequestDH;  
  DH_DFTResult     itsResultDH; 
  
  TH_Socket *itsTHProtoRequest;
  TH_Socket *itsTHProtoResult;
  bool      itsSynchronisity;
};

}


