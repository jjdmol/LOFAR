//# lofarcoordserver.cc: Deamon to handle coordinate conversions
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Includes
#include <Common/Net/Socket.h>
#include <Common/lofar_string.h>
#include <Coord/CoordClient.h>
#include <Coord/Endian.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Exceptions/Error.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace LOFAR;
using namespace casa;


// Initialize the deamon.
// The deamon initialization is based on 'Advanced Programming in the
// UNIX Environment" by W. Richard Stevens, section 13.3.
int initDeamon()
{
  // Fork to make process not a group leader.
  pid_t pid;
  if ( (pid = fork()) < 0) {
    return -1;
  } else if (pid != 0) {
    exit(0);
  }
  // Create a new session and fork again.
  setsid();         // become session leader
  if ( (pid = fork()) < 0) {
    return -1;
  } else if (pid != 0) {
    exit(0);
  }
  // Change working dir to avoid use of mounted file systems.
  chdir ("/");
  // Clear file mode creation mask.
  umask (0);
  return 0;
}

inline int getEndian (double* buf)
{
  return *((int*)(buf));
}
inline int getVersion (double* buf, bool swap)
{
  return CoordClient::getInt (buf+1, swap);
}

inline void setEndian (double* buf)
{
  *((int*)(buf)) = Endian().isLittleEndian();
}
inline void setVersion (double* buf, int version)
{
  *((int*)(buf+1)) = version;
}
inline void setNr (double* buf, int nr)
{
  *((int*)(buf)) = nr;
}

void sendError (Socket& socket)
{
  double buf[1];
  setNr (buf, 0);
  socket.writeBlocking (buf, sizeof(buf));
}

// Convert J2000 coordinates to AZEL.
void j2000ToAzel (Socket& socket, bool swap)
{
  double buf[3];
  if (socket.readBlocking (buf, 3*sizeof(double)) != 3*sizeof(double)) {
    sendError (socket);
    return;
  }
  int nrsky = CoordClient::getInt (buf, swap);
  int nrpos = CoordClient::getInt (buf+1, swap);
  int nrtime = CoordClient::getInt (buf+2, swap);
  int nrval = nrsky*nrpos*nrtime;
  int nrin = 2*nrsky + 3*nrpos + 2*nrtime;
  double* in = new double[nrin];
  socket.readBlocking (in, nrin*sizeof(double));
  if (swap) {
    Endian::swap (nrin, in);
  }
  double* out = new double[1 + 2*nrval];
  setNr (out, nrval);
  double* outData = out+1;
  const double* skyData = in;
  const double* posData = skyData + 2*nrsky;
  const double* timeData = posData + 3*nrpos;

  try {
    MeasFrame frame;
    MDirection dir;
    Quantity height(0, "m");
    Quantity angle1(0, "rad");
    Quantity angle2(0, "rad");
    for (int k=0; k<nrtime; k++) {
      frame.set (MEpoch(MVEpoch(timeData[2*k], timeData[2*k+1])));
      for (int j=0; j<nrpos; j++) {
        angle1.setValue (posData[3*j]);
        angle2.setValue (posData[3*j+1]);
        height.setValue (posData[3*j+2]);
        MVPosition pos(height, angle1, angle2);
        frame.set (MPosition(pos));
        MDirection::Convert conv (dir,
                                  MDirection::Ref (MDirection::AZEL, frame));
        for (int i=0; i<nrsky; i++) {
          angle1.setValue (skyData[2*i]);
          angle2.setValue (skyData[2*i+1]);
          MDirection sky(MVDirection(angle1, angle2), MDirection::J2000);
          MDirection res = conv(sky);
          Quantum<Vector<Double> > angles = res.getAngle();
          *outData++ = angles.getBaseValue()(0);
          *outData++ = angles.getBaseValue()(1);
        }
      }
    }
  }
  catch (AipsError& e) {
    LOG_ERROR(formatString("AipsError: %s", e.what()));
    return;
  }
  socket.writeBlocking (out, (1+2*nrval) * sizeof(double));
}

// Convert azimuth/elevation to J2000.
void azelToJ2000 (Socket& socket, bool swap)
{
  double buf[3];
  if (socket.readBlocking (buf, 3*sizeof(double)) != 3*sizeof(double)) {
    sendError (socket);
    return;
  }
  int nrsky = CoordClient::getInt (buf, swap);
  int nrpos = CoordClient::getInt (buf+1, swap);
  int nrtime = CoordClient::getInt (buf+2, swap);
  if (nrtime != 1) {
    if (nrsky != nrpos  ||  nrsky != nrtime) {
      sendError (socket);
      return;
    }
  } else if (nrpos != 1) {
    sendError (socket);
    return;
  }
  int nrval = nrsky;
  int nrin = 2*nrsky + 3*nrpos + 2*nrtime;
  double* in = new double[nrin];
  socket.readBlocking (in, nrin*sizeof(double));
  if (swap) {
    Endian::swap (nrin, in);
  }
  double* out = new double[1 + 2*nrval];
  setNr (out, nrval);
  double* outData = out+1;
  const double* skyData = in;
  const double* posData = skyData + 2*nrsky;
  const double* timeData = posData + 3*nrpos;

  try {
    MeasFrame frame;
    MDirection dir;
    Quantity height(0, "m");
    Quantity angle1(0, "rad");
    Quantity angle2(0, "rad");
    frame.set (MEpoch(MVEpoch(timeData[0], timeData[1])));
    angle1.setValue (posData[0]);
    angle2.setValue (posData[1]);
    height.setValue (posData[2]);
    MVPosition pos(height, angle1, angle2);
    frame.set (MPosition(pos));
    if (nrtime == 1) {
      frame.set (MEpoch(MVEpoch(timeData[0], timeData[1])));
      angle1.setValue (posData[0]);
      angle2.setValue (posData[1]);
      height.setValue (posData[2]);
      MVPosition pos(height, angle1, angle2);
      frame.set (MPosition(pos));
      MDirection::Ref ref(MDirection::AZEL, frame);
      MDirection::Convert conv (dir, MDirection::J2000);
      for (int i=0; i<nrsky; i++) {
        angle1.setValue (skyData[2*i]);
        angle2.setValue (skyData[2*i+1]);
        MDirection sky(MVDirection(angle1, angle2), ref);
        MDirection res = conv(sky);
        Quantum<Vector<Double> > angles = res.getAngle();
        *outData++ = angles.getBaseValue()(0);
        *outData++ = angles.getBaseValue()(1);
      }
    } else {
      for (int i=0; i<nrsky; i++) {
        frame.set (MEpoch(MVEpoch(timeData[2*i], timeData[2*i+1])));
        angle1.setValue (posData[2*i]);
        angle2.setValue (posData[2*i+1]);
        height.setValue (posData[2*i+2]);
        MVPosition pos(height, angle1, angle2);
        frame.set (MPosition(pos));
        MDirection::Ref ref(MDirection::AZEL, frame);
        MDirection::Convert conv (dir, MDirection::J2000);
        angle1.setValue (skyData[2*i]);
        angle2.setValue (skyData[2*i+1]);
        MDirection sky(MVDirection(angle1, angle2), ref);
        MDirection res = conv(sky);
        Quantum<Vector<Double> > angles = res.getAngle();
        *outData++ = angles.getBaseValue()(0);
        *outData++ = angles.getBaseValue()(1);
      }
    }
  }
  catch (AipsError& e) {
    LOG_ERROR(formatString("AipsError: %s", e.what()));
    return;
  }
  socket.writeBlocking (out, (1+2*nrval) * sizeof(double));
}

void handleConnection (Socket* socket)
{
  // Fork a child process to handle the connection.
  // The parent continues to accept other connections.
  pid_t pid;
  if ( (pid = fork()) < 0) {
    LOG_ERROR("Failed to fork a child process");
    return;
  } else if (pid != 0) {
    // Parent
    return;
  }
  LOG_DEBUG("Established connection and forked");
  socket->setBlocking();

  // This is the child process.
  // Determine version and endian format of client.
  Endian endian;
  double buf[2];
  // Read 2 doubles (for endian and version).
  ASSERT (socket->readBlocking (buf, 2*sizeof(double)) == 2*sizeof(double));
  // Determine if byte swapping is needed.
  bool swap = getEndian(buf) != endian.isLittleEndian();
  ASSERT (getVersion(buf, swap) == 1);
  // Send back the endian format and version of the server.
  setEndian (buf);
  setVersion (buf, 1);
  socket->writeBlocking (buf, 2*sizeof(double));

  // Read commands as long as we are connected.
  while (socket->isConnected()) {
    // Read 1 double (for command).
    if (socket->readBlocking (buf, sizeof(double)) == sizeof(double)) {
      int cmd = CoordClient::getInt (buf, swap);
      LOG_DEBUG(formatString("read command %d", cmd));
      switch (cmd) {
      case CoordClient::J2000ToAzel:
	j2000ToAzel (*socket, swap);
	break;
      case CoordClient::AzelToJ2000:
	azelToJ2000 (*socket, swap);
	break;
      case CoordClient::Disconnect:
	socket->shutdown (true, true);
	delete socket;
	LOG_DEBUG("Shutdown connection");
	exit(0);
      default:
	LOG_WARN(formatString("Unknown command %d", cmd));
	exit(1);
      }
    } else {
      sendError (*socket);
    }
  }
  socket->shutdown (true, true);
  delete socket;
  exit(0);
}


int main (int argc, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  ////  initDeamon();    // Do not start the process as a true deamon (yet).
  // Determine the port.
  string port = "31337";
  if (argc > 1) {
    port = argv[1];
  }

  // Create the main socket
  Socket mainSocket ("CoordConv", port);
  if (!mainSocket.ok()) {
    LOG_ERROR(formatString("Failed to create main socket: %s", 
                           mainSocket.errstr().c_str()));
    return 1;
  }

  while (true) {
    // See if there is a connection request.
    // If so, handle it in a child process.
    Socket* connSocket = mainSocket.accept();
    if (connSocket != 0) {
      LOG_DEBUG("Found connection");
      handleConnection (connSocket);
      delete connSocket;
    }
    // Wait (non-blocking) for any child.
    // This is needed to prevent a child from becoming a zombie when it
    // is finished.
    waitpid (-1, 0, WNOHANG);
    // Sleep 0.1 millisec.
    usleep(100);
  }
  
  return 0;
}
