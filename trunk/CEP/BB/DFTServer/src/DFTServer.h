//# DFTServer.h: DFT server application for use in combination with 
//#              BBS2 application.
//#
//# Copyright (C) 2004
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


#ifndef CEPFRAME_DFTSERVER_H
#define CEPFRAME_DFTSERVER_H

#include <CEPFrame/ApplicationHolder.h>

/**
  The DFTServer application is used in combination with the BBS2
  application. The DFT calculation formerly being integrated in the
  MNS object tree is now calculated uin the separate application. The
  main reasons to do so are to
  - allow for low-level MPI commands within this application
  (independent of the application containing the MNS objects)
  - allow for independent control of the DFT calculation tasks. This
  feature is especially interesting for deployment of thuis task on an
  external fast computer or GPU. 
*/

class DFTServer: public LOFAR::ApplicationHolder
{
public:
  virtual ~DFTServer();

  virtual void define (const LOFAR::KeyValueMap&);
  virtual void run (int nsteps);
  virtual void dump() const;
  virtual void quit();
};


#endif
