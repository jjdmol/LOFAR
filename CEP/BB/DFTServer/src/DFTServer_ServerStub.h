
#include <ACC/ParameterSet.h>
#include <PSS3/DH_InDFT.h>
#include <PSS3/DH_OutDFT.h>
#include <Transport/TH_Socket.h>

 
class DFTServer_ClientStub() 
{
public:
  DFTServer() {
    const ParameterSet myPS("params.ps");
    itsTHProtoIn = new TH_Socket(myPS.getString("DFTConnection.ServerHost"),   // sendhost
				 myPS.getString("DFTConnection.ServerHost"),   // recvhost
				 myPS.getString("DFTConnection.ServerPort")    // port
                        );    
    itsTHProtoOut = new TH_Socket(myPS.getString("DFTConnection.ServerHost"),   // sendhost
				  myPS.getString("DFTConnection.ServerHost"),   // recvhost
				  myPS.getString("DFTConnection.ServerPort")    // port
                        );    
    
  };
  ~DFTServer() {};
  
  DH_InDFT      itsInDH();  
  DH_OutDFT     itsOutDH(); 
  
  TH_Socket *itsTHProtoIn;
  TH_Socket *itsTHProtoOut;
  bool      itsSynchronisity;
}


