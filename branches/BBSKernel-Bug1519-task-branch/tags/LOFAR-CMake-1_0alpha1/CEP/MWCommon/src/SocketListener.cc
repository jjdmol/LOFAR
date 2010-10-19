//# SocketListener.cc: Class that creates a socket and accepts connections
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/SocketListener.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>


namespace LOFAR { namespace CEP {

  SocketListener::SocketListener (const std::string& port)
    : itsConnSocket (new LOFAR::Socket("mwsck", port))
  {}

  SocketConnection::ShPtr SocketListener::accept()
  {
    LOFAR::Socket* socket = itsConnSocket->accept();
    SocketConnection::ShPtr dataConn(new SocketConnection(socket));
    int status = itsConnSocket->errcode();
    ASSERTSTR (socket  &&  status == LOFAR::Socket::SK_OK,
                 "SocketConnection server did not accept on host "
                 << itsConnSocket->host() << ", port " << itsConnSocket->port()
                 << ", LOFAR::Socket status " << status << ' '
                 << itsConnSocket->errstr());
    ASSERT (dataConn->isConnected());
    return dataConn;
  }

}} // end namespaces
