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
#include <GCF/GCF_PVString.h>

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
  bool retVal(true);
  if (msg.getArrayLength() < 2) return false;
  CharString qualifier = (CharString&) *((TextVar*) msg[1]);
  if (qualifier == "m") // message
  {   
    if (msg.getArrayLength() < 6) return false;
          
    CharString signal = (CharString &) *((TextVar*) msg[4]);
    UIntegerVar pvssSeqnr(atoi((const char*) *((TextVar*) msg[5])));
    GCFEvent* pEvent(0);
    if (signal == "l")
    {
      PALoadPropSetEvent request;
         
      request.seqnr = pvssSeqnr;
      request.scope = (const char*) *((TextVar*) msg[6]);
      pEvent = &request;
    }
    else if (signal == "ul")
    {
      PAUnloadPropSetEvent request;
         
      request.seqnr = pvssSeqnr;
      request.scope = (const char*) *((TextVar*) msg[6]);
      pEvent = &request;
    }
    else if (signal == "conf")
    {
      PAConfPropSetEvent request;
         
      request.seqnr = pvssSeqnr;
      request.scope = (const char*) *((TextVar*) msg[6]);
      if (msg.getArrayLength() < 7) return false;
      request.apcName = (const char*) *((TextVar*) msg[7]);
      pEvent = &request;
    }
    else 
    {
      retVal = false;
    }

    if (retVal)
    {
      GCFPVString portId((const char*)*((TextVar*) msg[2]));
      GCFPVString portAddr((const char*)*((TextVar*) msg[3]));
      unsigned int packsize;
      void* buf = pEvent->pack(packsize);
    
      unsigned int newBufSize = 1 + portId.getSize() + portAddr.getSize() + packsize;
      unsigned char* newBuf = new unsigned char[newBufSize];
      newBuf[0] = 'm'; // just a message
      unsigned int bytesPacked = 1;
      bytesPacked += portId.pack((char*) newBuf + bytesPacked);
      bytesPacked += portAddr.pack((char*) newBuf + bytesPacked);
      memcpy(newBuf + bytesPacked, buf, packsize);
  
      BlobVar newBlob(newBuf, newBufSize, PVSS_TRUE);
      gcfEvent = newBlob;
      delete [] newBuf;
    }
  }
  else if (qualifier == "d") // disconnected
  {
    GCFPVString portId((const char*)*((TextVar*) msg[2]));
    unsigned int newBufSize = 1 + portId.getSize();
    unsigned char* newBuf = new unsigned char[newBufSize];
    newBuf[0] = 'd'; // just a message
    unsigned int bytesPacked = 1;
    bytesPacked += portId.pack((char*) newBuf + bytesPacked);

    BlobVar newBlob(newBuf, newBufSize, PVSS_TRUE);
    gcfEvent = newBlob;
    delete [] newBuf;    
  }
  else
  {
    retVal = false;
  }
  return retVal;
}

bool GPAExternHdl::gpaConvertGCFEventToMsg(const BlobVar& gcfEvent, DynVar& msg)
{
  bool retVal(true);
  msg.clear();
  _msgBuffer = gcfEvent.getValue().getData();
  _bytesLeft = gcfEvent.getValue().getLen();
  unsigned int bytesRead = 1;
  GCFPVString portId;
  bytesRead += portId.unpack((char*) _msgBuffer + bytesRead);
  switch (_msgBuffer[0])
  {
    case 'd': // disconnect event
    {
      TextVar qualifier("d");
      msg.append(qualifier);

      TextVar pvssPortId(portId.getValue().c_str());
      msg.append(pvssPortId);
      break;
    } 
    case 'm': // message
    {
      TextVar qualifier("m");
      msg.append(qualifier);

      TextVar pvssPortId(portId.getValue().c_str());
      msg.append(pvssPortId);

      GCFPVString portAddr;
      bytesRead += portAddr.unpack((char *)_msgBuffer + bytesRead);
      TextVar pvssPortAddr(portAddr.getValue().c_str());
      msg.append(pvssPortAddr);
      
      GCFEvent e;
      // expects and reads signal
      if (recv(&e.signal, sizeof(e.signal)) != sizeof(e.signal)) 
      {
        // don't continue with receiving
      }
      // expects and reads length
      else if (recv(&e.length, sizeof(e.length)) != sizeof(e.length))
      {
        // don't continue with receiving
      }  
      // reads payload if specified
      else if (e.length > 0)
      {
        GCFEvent* full_event = 0;
        char* event_buf = new char[sizeof(e) + e.length];
        full_event = (GCFEvent*)event_buf;
        memcpy(event_buf, &e, sizeof(e));
    
        // read the payload right behind the just memcopied basic event structure
        if (recv(event_buf + sizeof(e), e.length) > 0)
        {          
          // dispatchs an event with just received params
          encodeEvent(*full_event, msg);
        }    
        delete [] event_buf;
      }
      // dispatchs an event without params
      else
      {
        encodeEvent(e, msg);
      }
            
      break;
    }
    default:
      retVal = false;
      break;
  }
  
  return retVal;
}

ssize_t GPAExternHdl::recv (void* buf, size_t count)
{
  if (_bytesLeft > 0)
  {
    if (count > _bytesLeft) count = _bytesLeft;
    memcpy(buf, _msgBuffer, count);
    _msgBuffer += count;
    _bytesLeft -= count;
    return count;
  }
  else
  {
    return 0;
  }
}

void GPAExternHdl::encodeEvent(GCFEvent& e, DynVar& msg)
{
  switch (e.signal)
  {
    case PA_PROP_SET_LOADED:
    {
      PAPropSetLoadedEvent response(e);
      TextVar newMsgItem("loaded");
      msg.append(newMsgItem);
      string seqNrString = formatString("%d", response.seqnr);
      newMsgItem = seqNrString.c_str();
      msg.append(newMsgItem);
      newMsgItem = (response.result == PA_NO_ERROR ? "OK" : "failed");
      msg.append(newMsgItem);
      break;
    }
    case PA_PROP_SET_UNLOADED:
    {
      PAPropSetUnloadedEvent response(e);
      TextVar newMsgItem("unloaded");
      msg.append(newMsgItem);
      string seqNrString = formatString("%d", response.seqnr);
      newMsgItem = seqNrString.c_str();
      msg.append(newMsgItem);
      newMsgItem = (response.result == PA_NO_ERROR ? "OK" : "failed");
      msg.append(newMsgItem);
      break;
    }
    case PA_PROP_SET_CONF:
    {
      PAPropSetConfEvent response(e);
      TextVar newMsgItem("configured");
      msg.append(newMsgItem);
      string seqNrString = formatString("%d", response.seqnr);
      newMsgItem = seqNrString.c_str();
      msg.append(newMsgItem);
      newMsgItem = (response.result == PA_NO_ERROR ? "OK" : "failed");
      msg.append(newMsgItem);
      newMsgItem = response.apcName.c_str();
      msg.append(newMsgItem);
      break;
    }
    case PA_PROP_SET_GONE:
    {
      PAPropSetGoneEvent indication(e);
      TextVar newMsgItem("gone");
      msg.append(newMsgItem);
      newMsgItem = indication.scope.c_str();
      msg.append(newMsgItem);
      break;
    }    
  }
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
