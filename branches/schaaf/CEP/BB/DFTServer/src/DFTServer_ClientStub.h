

#include <PSS3/DH_InDFT.h>
#include <PSS3/DH_OutDFT.h>
#include <Transport/TH_Socket.h>

 
class DFTServer_ClientStub() 
{
  DFTServer() {};
  ~DFTServer() {};

 private:
  DH_InDFT      itsInDH();   //todo: need to set ID...
  DH_OutDFT     itsOutDH();  //todo: need to set ID...

  TH_Socket itsTHProto(); 
  bool      itsSynchronisity;
}


