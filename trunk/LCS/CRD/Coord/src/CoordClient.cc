//# CoordClient.cc: Class to handle coordinate conversions in a server
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

#include <Coord/CoordClient.h>
#include <Coord/SkyCoord.h>
#include <Coord/EarthCoord.h>
#include <Coord/TimeCoord.h>
#include <Coord/Endian.h>
#include <Common/LofarLogger.h>


CoordClient::CoordClient (const string& host, const string& port)
: itsNrStart (4),
  itsSocket  ("CoordConv", host, port),
  itsBuffer  (0)
{
  itsSocket.setBlocking();
  int sts = 0;
  while (sts == 0) {
    sts = itsSocket.connect();
  }
  ASSERTSTR (sts == 1,
	     "CoordClient could not connect to server on host " << host
	     << ", port " << port);
  // Make sure the buffer is long enough for a single conversion
  // which requires 7 doubles.
  allocate(7);
  // Send the endian format and version to the server
  // and receive the ones from the server.
  int isLittle = Endian().isLittleEndian();
  putEndian (isLittle);
  putVersion (1);
  itsSocket.writeBlocking (itsBuffer, 2*sizeof(double));
  itsSocket.readBlocking (itsBuffer, 2*sizeof(double));
  // Determine if data has to be swapped.
  itsSwap = getEndian() != isLittle;
  ASSERT (getVersion() == 1);
}

CoordClient::~CoordClient()
{
  putCommand (Disconnect);
  itsSocket.writeBlocking (itsBuffer, sizeof(double));
  itsSocket.shutdown (true, true);
  deallocate();
}

int CoordClient::getInt (double* buf, bool swap)
{
  int val = *((int*)(buf));
  if (swap) {
    Endian::swapInt (&val);
  }
  return val;
}

void CoordClient::allocate (int nr)
{
  deallocate();
  itsPosBufNr = nr;
  itsBuffer = new double[nr+itsNrStart];
  itsPosBuffer = itsBuffer+itsNrStart;
  // Initialize the start of the buffer (to avoid purify errors).
  for (int i=0; i<itsNrStart; i++) {
    itsBuffer[i] = 0;
  }
}

void CoordClient::deallocate()
{
  delete [] itsBuffer;
  itsBuffer = 0;
}

SkyCoord CoordClient::j2000ToAzel (const SkyCoord& radec,
				   const EarthCoord& pos,
				   const TimeCoord& time)
{
  itsPosBuffer[0] = radec.angle1();
  itsPosBuffer[1] = radec.angle2();
  itsPosBuffer[2] = pos.longitude();
  itsPosBuffer[3] = pos.latitude();
  itsPosBuffer[4] = pos.height();
  itsPosBuffer[5] = time.getDay();
  itsPosBuffer[6] = time.getFraction();
  putNrSky   (1);
  putNrEarth (1);
  putNrTime  (1);
  putCommand (J2000ToAzel);
  itsSocket.writeBlocking (itsBuffer, (7+itsNrStart)*sizeof(double));
  itsSocket.readBlocking (itsBuffer, sizeof(double));
  ASSERT (getInt(itsBuffer) == 1);
  itsSocket.readBlocking (itsPosBuffer, 2*sizeof(double));
  return SkyCoord (itsPosBuffer[0], itsPosBuffer[1]);
}

vector<SkyCoord> CoordClient::j2000ToAzel (const vector<SkyCoord>& radec,
					   const EarthCoord& pos,
					   const TimeCoord& time)
{
  return j2000ToAzel (radec,
		      vector<EarthCoord>(1, pos),
		      vector<TimeCoord>(1, time));
}

vector<SkyCoord> CoordClient::j2000ToAzel (const SkyCoord& radec,
					   const vector<EarthCoord>& pos,
					   const TimeCoord& time)
{
  return j2000ToAzel (vector<SkyCoord>(1, radec),
		      pos,
		      vector<TimeCoord>(1, time));
}

vector<SkyCoord> CoordClient::j2000ToAzel (const SkyCoord& radec,
					   const EarthCoord& pos,
					   const vector<TimeCoord>& time)
{
  return j2000ToAzel (vector<SkyCoord>(1, radec),
		      vector<EarthCoord>(1, pos),
		      time);
}

vector<SkyCoord> CoordClient::j2000ToAzel (const vector<SkyCoord>& radec,
					   const vector<EarthCoord>& pos,
					   const vector<TimeCoord>& time)
{
  vector<SkyCoord> result;
  if (radec.size() == 0  ||  pos.size() == 0  || time.size() == 0) {
    return result;
  }
  int nr = 2*radec.size() + 3*pos.size() + 2*time.size();
  int nrval = radec.size()*pos.size()*time.size();
  int sz = std::max(nr, 2*nrval);
  if (sz > itsPosBufNr) {
    allocate (sz);
  }
  double* buf = itsPosBuffer;
  for (vector<SkyCoord>::const_iterator iter = radec.begin();
       iter != radec.end();
       ++iter) {
    *buf++ = iter->angle1();
    *buf++ = iter->angle2();
  }
  for (vector<EarthCoord>::const_iterator iter = pos.begin();
       iter != pos.end();
       ++iter) {
    *buf++ = iter->longitude();
    *buf++ = iter->latitude();
    *buf++ = iter->height();
  }
  for (vector<TimeCoord>::const_iterator iter = time.begin();
       iter != time.end();
       ++iter) {
    *buf++ = iter->getDay();
    *buf++ = iter->getFraction();
  }
  putNrSky   (radec.size());
  putNrEarth (pos.size());
  putNrTime  (time.size());
  putCommand (J2000ToAzel);
  itsSocket.writeBlocking (itsBuffer, (nr+itsNrStart)*sizeof(double));
  itsSocket.readBlocking (itsBuffer, sizeof(double));
  ASSERT (getInt(itsBuffer) == nrval);
  itsSocket.readBlocking (itsPosBuffer, 2*nrval*sizeof(double));
  result.reserve (nrval);
  for (int i=0; i<nrval; i++) {
    result.push_back (SkyCoord (itsPosBuffer[2*i], itsPosBuffer[2*i+1]));
  }
  return result;
}


SkyCoord CoordClient::azelToJ2000 (const SkyCoord& azel,
				   const EarthCoord& pos,
				   const TimeCoord& time)
{
  itsPosBuffer[0] = azel.angle1();
  itsPosBuffer[1] = azel.angle2();
  itsPosBuffer[2] = pos.longitude();
  itsPosBuffer[3] = pos.latitude();
  itsPosBuffer[4] = pos.height();
  itsPosBuffer[5] = time.getDay();
  itsPosBuffer[6] = time.getFraction();
  putNrSky   (1);
  putNrEarth (1);
  putNrTime  (1);
  putCommand (AzelToJ2000);
  itsSocket.writeBlocking (itsBuffer, (7+itsNrStart)*sizeof(double));
  itsSocket.readBlocking (itsBuffer, sizeof(double));
  ASSERT (getInt(itsBuffer) == 1);
  itsSocket.readBlocking (itsPosBuffer, 2*sizeof(double));
  return SkyCoord (itsPosBuffer[0], itsPosBuffer[1]);
}

vector<SkyCoord> CoordClient::azelToJ2000 (const vector<SkyCoord>& azel,
					   const EarthCoord& pos,
					   const TimeCoord& time)
{
  vector<SkyCoord> result;
  if (azel.size() == 0) {
    return result;
  }
  int nr = 2*azel.size() + 3 + 2;
  int nrval = azel.size();
  if (nr > itsPosBufNr) {
    allocate (nr);
  }
  double* buf = itsPosBuffer;
  for (vector<SkyCoord>::const_iterator iter = azel.begin();
       iter != azel.end();
       ++iter) {
    *buf++ = iter->angle1();
    *buf++ = iter->angle2();
  }
  buf[0] = pos.longitude();
  buf[1] = pos.latitude();
  buf[2] = pos.height();
  buf[3] = time.getDay();
  buf[4] = time.getFraction();
  putNrSky   (azel.size());
  putNrEarth (1);
  putNrTime  (1);
  putCommand (AzelToJ2000);
  itsSocket.writeBlocking (itsBuffer, (nr+itsNrStart)*sizeof(double));
  itsSocket.readBlocking (itsBuffer, sizeof(double));
  ASSERT (getInt(itsBuffer) == nrval);
  itsSocket.readBlocking (itsPosBuffer, 2*nrval*sizeof(double));
  result.reserve (nrval);
  for (int i=0; i<nrval; i++) {
    result.push_back (SkyCoord (itsPosBuffer[2*i], itsPosBuffer[2*i+1]));
  }
  return result;
}

vector<SkyCoord> CoordClient::azelToJ2000 (const vector<SkyCoord>& azel,
					   const vector<EarthCoord>& pos,
					   const vector<TimeCoord>& time)
{
  ASSERT (azel.size() == pos.size()  &&  azel.size() == time.size());
  vector<SkyCoord> result;
  if (azel.size() == 0) {
    return result;
  }
  int nr = (2+3+2)*azel.size();
  int nrval = azel.size();
  if (nr > itsPosBufNr) {
    allocate (nr);
  }
  double* buf = itsPosBuffer;
  for (vector<SkyCoord>::const_iterator iter = azel.begin();
       iter != azel.end();
       ++iter) {
    *buf++ = iter->angle1();
    *buf++ = iter->angle2();
  }
  for (vector<EarthCoord>::const_iterator iter = pos.begin();
       iter != pos.end();
       ++iter) {
    *buf++ = iter->longitude();
    *buf++ = iter->latitude();
    *buf++ = iter->height();
  }
  for (vector<TimeCoord>::const_iterator iter = time.begin();
       iter != time.end();
       ++iter) {
    *buf++ = iter->getDay();
    *buf++ = iter->getFraction();
  }
  putNrSky   (azel.size());
  putNrEarth (pos.size());
  putNrTime  (time.size());
  putCommand (AzelToJ2000);
  itsSocket.writeBlocking (itsBuffer, (nr+itsNrStart)*sizeof(double));
  itsSocket.readBlocking (itsBuffer, sizeof(double));
  ASSERT (getInt(itsBuffer) == nrval);
  itsSocket.readBlocking (itsPosBuffer, 2*nrval*sizeof(double));
  result.reserve (nrval);
  for (int i=0; i<nrval; i++) {
    result.push_back (SkyCoord (itsPosBuffer[2*i], itsPosBuffer[2*i+1]));
  }
  return result;
}
