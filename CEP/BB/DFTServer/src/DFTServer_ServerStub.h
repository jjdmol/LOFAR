
#include <ACC/ParameterSet.h>
#include <PSS3/DH_DFTRequest.h>
#include <PSS3/DH_DFTResult.h>
#include <Transport/TH_Socket.h>

namespace LOFAR { 

class DFTServer_ServerStub
{
public:
  DFTServer_ServerStub() {
    const ParameterSet myPS("params.ps");
    itsTHProtoIn = new TH_Socket(myPS.getString("DFTConnection.ServerHost"),   // sendhost
				 myPS.getString("DFTConnection.ServerHost"),   // recvhost
				 myPS.getInt("DFTConnection.ServerPort")    // port
                        );    
    itsTHProtoOut = new TH_Socket(myPS.getString("DFTConnection.ServerHost"),   // sendhost
				  myPS.getString("DFTConnection.ServerHost"),   // recvhost
				  myPS.getInt("DFTConnection.ServerPort")    // port
                        );    
    
  };
  ~DFTServer_ServerStub() {};
  
  DH_DFTRequest      itsRequestDH;  
  DH_DFTResult     itsResultDH; 
  
  TH_Socket *itsTHProtoIn;
  TH_Socket *itsTHProtoOut;
  bool      itsSynchronisity;
};

}


