// Ctrl extension for TCP communication

# ifdef _WIN32
#   include <winsock2.h>
# else
#   include  <netdb.h>
#   include  <sys/socket.h>
#   include  <netinet/in.h>
#   include  <arpa/inet.h>
#   include  <errno.h>
#   include  <sys/time.h>
#   include  <sys/types.h>
#   include  <unistd.h>
# endif

#include "GPA_ExternHdl.h"
#include <PA_Protocol.ph>

#include <stdio.h>
#include <CharString.hxx>
#include <BlobVar.hxx>
#include <CtrlExpr.hxx>
#include <ExprList.hxx>
#include <CtrlThread.hxx>
#include <IdExpr.hxx>
#include <Variable.hxx>
#include <AnyTypeVar.hxx>
#include <BitVar.hxx>
#include <TextVar.hxx>
#include <UIntegerVar.hxx>
#include <ExternData.hxx>
#include <ErrClass.hxx>
#include <BCTime.h>


// Function description
FunctionListRec GPAExternHdl::fnList[] = 
{
  //  Return-Value    function name        parameter list               true == thread-save
  //  -------------------------------------------------------------------------------------
    { BIT_VAR, "gpaConvertMsgToGCFEvent", "(dyn_string msg,  blob& event)"     , true},
    { BIT_VAR, "gpaConvertGCFEventToMsg", "(blob sendData, dyn_string& msg)"   , true},
  };

// Remars:
// tcpConnect: Argument portNr could be int
// tcpSend:    Argument sendData could be string
// tcpRecv:    Arguemnt recvData could be string


BaseExternHdl *newExternHdl(BaseExternHdl *nextHdl)
{
  // Calculate number of functions defined
  PVSSulong funcCount = sizeof(GPAExternHdl::fnList)/sizeof(GPAExternHdl::fnList[0]);  
  
  // now allocate the new instance
  GPAExternHdl *hdl = new GPAExternHdl(nextHdl, funcCount, GPAExternHdl::fnList);

  return hdl;
}


//------------------------------------------------------------------------------
// This function is called every time we use tcpXXX in a Ctrl script.
// The argument is an aggregaton of function name, function number, 
// the arguments passed to the Ctrl function, the "thread" context 
// and user defined data.
const Variable* GPAExternHdl::execute(ExecuteParamRec &param)
{
  // We return a pointer to a static variable. 
  static BitVar bitVar;

  // A pointer to one "argument". Any input argument may be an exprssion
  // like "a ? x : y" instead of a simple value.
  CtrlExpr* expr;
  // A generic pointer to the input / output value
  const Variable* pVar;

  // switch / case on function numbers
  switch (param.funcNum)
  {
    case F_gpaConvertMsgToGCFEvent:
    {
      // Forget all errors so far. We generate our own errors
      param.thread->clearLastError();

    	// Check for first parameter "msg" (string)
      if (!param.args || !(expr = param.args->getFirst()) ||
          !(pVar = expr->evaluate(param.thread)) || (pVar->isA() != DYNTEXT_VAR)
         )
      {
        ErrClass  errClass(
          ErrClass::PRIO_WARNING, ErrClass::ERR_PARAM, ErrClass::ARG_MISSING,
          "GPAExternHdl", "execute", "missing or wrong argument gpaConvertMsgToGCFEvent");

        // Remember error message. 
        // In the Ctrl script use getLastError to retreive the error message
        param.thread->appendLastError(&errClass);

        // Return error
        bitVar.setValue (PVSS_FALSE); 
        return &bitVar;    // error ->return
      }

      // Get first parameter
      DynVar msg;
      msg = *pVar;        // Assignment calls the appropriate operator

      // Check for next parameter "gcfEvent" (int or unsgined int)
      if (!param.args || !(expr = param.args->getNext()) ||
          !(pVar = expr->evaluate(param.thread)) ||
          (pVar->isA() != BLOB_VAR) 
         )
      {
        ErrClass  errClass(
          ErrClass::PRIO_WARNING, ErrClass::ERR_PARAM, ErrClass::ARG_MISSING,
          "GPAExternHdl", "gpaConvertMsgToGCFEvent", "missing or wrong argument parameter");

        // Remember error message. 
        // In the Ctrl script use getLastError to retreive the error message
        param.thread->appendLastError(&errClass);

        // Return (-1) to indicate error
        bitVar.setValue (PVSS_FALSE); 
        return &bitVar;    // error ->return
      }
      
      bool retVal = gpaConvertMsgToGCFEvent(msg, *((BlobVar* )pVar));

      bitVar.setValue ((retVal ? PVSS_TRUE : PVSS_FALSE)); 
  	  return &bitVar;    
    }

    case F_gpaConvertGCFEventToMsg:
    {
      // Check for parameter GCFEvent (blob)
      if (!param.args || !(expr = param.args->getFirst()) ||
          !(pVar = expr->evaluate(param.thread)) || (pVar->isA() != BLOB_VAR)
         )
      {
        ErrClass  errClass(
          ErrClass::PRIO_WARNING, ErrClass::ERR_PARAM, ErrClass::ARG_MISSING,
          "GPAExternHdl", "execute", "missing or wrong argument");

        // Remember error message. 
        // In the Ctrl script use getLastError to retreive the error message
        param.thread->appendLastError(&errClass);

        // Return (-1) to indicate an error
        bitVar.setValue (PVSS_TRUE); 
        return &bitVar;    // error ->return
      }

      BlobVar gcfEvent;
      gcfEvent = *pVar;

      // Check for parameter msg (string)
      // This parameter is an output value (it receives our data),
      // so get a pointer via the getTarget call.
      if (!param.args || !(expr = param.args->getNext()) ||
          !(pVar = expr->getTarget(param.thread)) || 
          (pVar->isA() != DYNTEXT_VAR)
         )
      {
        ErrClass  errClass(
          ErrClass::PRIO_WARNING, ErrClass::ERR_PARAM, ErrClass::ARG_MISSING,
          "GPAExternHdl", "gpaConvertGCFEventToMsg", "missing or wrong argument data");

        // Remember error message. 
        // In the Ctrl script use getLastError to retreive the error message
        param.thread->appendLastError(&errClass);

        // Return (-1) to indicate error
        bitVar.setValue (PVSS_FALSE); 
        return &bitVar;    // error ->return
      }

      
      bool retVal = gpaConvertGCFEventToMsg(gcfEvent, *((DynVar* )pVar));

      bitVar.setValue ((retVal ? PVSS_TRUE : PVSS_FALSE)); 
      return &bitVar; 
    }


    // May never happen, but make compilers happy
    default: 
      bitVar.setValue(PVSS_FALSE);
      return &bitVar;
  }

  return &bitVar;
}


// -----------------------------------------------------------------------
// Utilitiy functions
bool GPAExternHdl::gpaConvertMsgToGCFEvent(const DynVar& msg, BlobVar& gcfEvent)
{
  CharString qualifier = (CharString&) *((TextVar*) msg[0]);
  if (qualifier == "m") // message
  {   
    CharString signal = (CharString &) *((TextVar*) msg[1]);
    UIntegerVar pvssSeqnr(atoi((const char*) *((TextVar*) msg[4])));
    GCFEvent* pEvent(0);
    if (signal == "l")
    {
      PALoadPropSetEvent request;
         
      request.seqnr = pvssSeqnr;
      request.scope = (const char*) *((TextVar*) msg[5]);
      pEvent = &request;
    }
    else if (signal == "ul")
    {
      PAUnloadPropSetEvent request;
         
      request.seqnr = pvssSeqnr;
      request.scope = (const char*) *((TextVar*) msg[5]);
      pEvent = &request;
    }
    else if (signal == "conf")
    {
      PAConfPropSetEvent request;
         
      request.seqnr = pvssSeqnr;
      request.scope = (const char*) *((TextVar*) msg[5]);
      request.apcName = (const char*) *((TextVar*) msg[6]);
      pEvent = &request;
    }

    CharString portId((CharString&)*((TextVar*) msg[2]));
    CharString portAddr((CharString&)*((TextVar*) msg[3]));
    unsigned int packsize;
    void* buf = pEvent->pack(packsize);
  
    unsigned int newBufSize = 1 + 4 + portId.len() + 4 + portAddr.len() + packsize;
    char* newBuf = new char[newBufSize];
    newBuf[0] = 'm'; // just a message
    unsigned int bytesPacked = 1;
    memcpy(newBuf + bytesPacked, portId.len(), 4);
    bytesPacked += 4;
    memcpy(newBuf + bytesPacked, (const char*) portId, portId.len());
    bytesPacked += portId.len();
    
    memcpy(newBuf + bytesPacked, portAddr.len(), 4);
    bytesPacked += 4;
    memcpy(newBuf + bytesPacked, (const char*) portAddr, portAddr.len());
    bytesPacked += portAddr.len();
    memcpy(newBuf + bytesPacked, buf, packsize);

    BlobVar newBlob(newBuf, newBufSize, PVSS_TRUE);
    gcfEvent = newBlob;
    delete [] newBuf;
  }
  else if (qualifier == "d") // disconnected
  {
    
  }
}

bool GPAExternHdl::gpaConvertGCFEventToMsg(const BlobVar& gcfEvent, DynVar& msg)
{
}



// DLLInit Funktion fuer Win NT
// Initialize / DeInit winsock
# ifdef _WIN32
BOOL  WINAPI  DllInit(HINSTANCE, DWORD dwReason, LPVOID)
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    // Initialize winsock, return FALSE on error
 	  WSADATA WsaData;
	  int err;
	
	  err = WSAStartup(0x0101, &WsaData);
	  if (err == SOCKET_ERROR)
      return FALSE;
  }
  else if (dwReason == DLL_PROCESS_DETACH)
    WSACleanup();

  return TRUE;
}
# endif
