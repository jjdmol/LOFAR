//# CoordClient.h: Class to handle coordinate conversions in a server
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

#if !defined(COORD_COORDCLIENT_H)
#define COORD_COORDCLIENT_H

//# Includes
#include <Common/Net/Socket.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
using LOFAR::Socket;

// Forward Declarations
class SkyCoord;
class EarthCoord;
class TimeCoord;


class CoordClient
{
public:
  // Make a client and let it connect to the coordinates server on
  // the given host and port.
  explicit CoordClient (const string& host, const string& port = "31337");

  // The destructor closes the connection.
  ~CoordClient();

  // Convert the given equatorial J2000 sky coordinate to azimuth/elevation
  // (in radians) for the given earth position and time.
  SkyCoord j2000ToAzel (const SkyCoord& radec,
			const EarthCoord& pos,
			const TimeCoord& time);

  // Convert a series of sky coordinates.
  vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
				const EarthCoord& pos,
				const TimeCoord& time);

  // Convert a sky coordinate for a series of earth positions.
  vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
				const vector<EarthCoord>& pos,
				const TimeCoord& time);

  // Convert a sky coordinate for a series of times.
  vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
				const EarthCoord& pos,
				const vector<TimeCoord>& time);

  // Convert a series of sky coordinates for a series of earth positions
  // and times.
  // The output vector contains radec.size() * pos.size() * time.size()
  // elements which can be seen as a cube with shape [nsky,npos,ntime]
  // in Fortran order (thus with sky as the most rapidly varying axis).
  vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
				const vector<EarthCoord>& pos,
				const vector<TimeCoord>& time);

  // Convert an azimuth/elevation for the given earth position and time
  // to ra/dec.
  SkyCoord azelToJ2000 (const SkyCoord& azel,
			const EarthCoord& pos,
			const TimeCoord& time);

  // Convert a series of azimuth/elevations for the given earth position
  // and time to ra/dec. The output vector has the same length as the
  // input azel vector.
  vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
				const EarthCoord& pos,
				const TimeCoord& time);

  // Convert a series of azimuth/elevations for the given earth positions
  // and times to ra/dec. The output vector has the same length as the
  // input azel vector.
  // All input vectors must have the same length.
  // The difference with the function above is that here each azel has
  // its own earth position and time.
  vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
				const vector<EarthCoord>& pos,
				const vector<TimeCoord>& time);

  // Define the possible commands sent to the server.
  enum Command {
    J2000ToAzel,
    AzelToJ2000,
    Disconnect
  };

  // Get an int value from the buffer and swap bytes as needed.
  static int getInt (double* buf, bool swap);

private:
  // Make the buffer big enough for nr data doubles.
  void allocate (int nr);

  // Deallocate the buffer.
  void deallocate();

  // Get an int value (in local format) from the buffer.
  int getInt (double* buf) const
    { return getInt (buf, itsSwap); }

  // Put/get the endian format in the buffer.
  void putEndian (int endianFormat)
    { *((int*)(itsBuffer)) = endianFormat; }
  int getEndian() const
    { return *((int*)(itsBuffer)); }

  // Put/get the communication version in the buffer.
  void putVersion (int version)
    { *((int*)(itsBuffer+1)) = version; }
  int getVersion() const
    { return getInt (itsBuffer+1); }

  // Put the command into the buffer.
  void putCommand (int command)
    { *((int*)(itsBuffer)) = command; }

  // Put the number of coordinates into the buffer.
  void putNrSky (int nr)
    { *((int*)(itsBuffer+1)) = nr; }
  void putNrEarth (int nr)
    { *((int*)(itsBuffer+2)) = nr; }
  void putNrTime (int nr)
    { *((int*)(itsBuffer+3)) = nr; }


  const int itsNrStart;  // nr of elements used for control at start of buffer
  Socket  itsSocket;
  int     itsPosBufNr;
  double* itsPosBuffer;
  double* itsBuffer;
  bool    itsSwap;
};


#endif
