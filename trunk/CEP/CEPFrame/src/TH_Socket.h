//# TH_Socket.h: POSIX Socket based transportat holder
//#
//# Copyright (C) 2000, 2001
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

#ifndef CEPFRAME_TH_MEM_BL_H
#define CEPFRAME_TH_MEM_BL_H

#include <lofar_config.h>

#include "CEPFrame/TransportHolder.h"
#include <Common/lofar_map.h>
#include <pthread.h>

namespace LOFAR
{

class TH_Socket: public TransportHolder
{
public:
  TH_Socket();
  virtual ~TH_Socket();

  virtual TH_Socket* make() const;

  virtual bool recv(void* buf, int nbytes, int source, int tag);
  virtual bool send(void* buf, int nbytes, int destination, int tag);

  virtual string getType() const;

  virtual bool connectionPossible(int srcRank, int dstRank) const;
  virtual bool isBlocking() const { return true; }

  static TH_Socket proto;
  
  static void init (int argc, const char *argv[]);
  static void finalize();
  static void waitForBroadCast();
  static void waitForBroadCast (unsigned long& aVar);
  static void sendBroadCast (unsigned long timeStamp);
  static int  getCurrentRank();
  static int  getNumberOfNodes();
  static void synchroniseAllProcesses();

 private:

};

}

#endif
