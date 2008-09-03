//#  MySocketExample.cc: Test program for socket transer using tinyCEP
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <MySocketExample.h>
#include <Transport/Connection.h>

namespace LOFAR
{

  MySocketExample::MySocketExample(int ninput, int noutput, bool sender)
    :  itsWH      (0),
       itsNinput  (ninput),
       itsNoutput (noutput),
       itsIsSender(sender),
       itsConn    (0),
       itsTH      (0)
  {
  }
      
  MySocketExample::~MySocketExample() {
    delete itsWH;
    delete itsConn;
    delete itsTH;
  }

  void MySocketExample::define (const KeyValueMap&) {

    string service("8192");
    // create the work holders
    if (itsIsSender) {

      itsWH = new WH_Example("WH_ExampleS", 1, 1, 10);

      // Create a server socket
      itsTH = new TH_Socket(service, false); 
      itsConn = new Connection("SendConn", itsWH->getDataManager().getOutHolder(0),
			       0, itsTH, false);

      itsWH->getDataManager().setOutConnection(0, itsConn);


    } else {

      itsWH = new WH_Example("WH_ExampleS", 1, 1, 10);

      // Create a client socket
      itsTH = new TH_Socket("127.0.0.1", service, false);
      itsConn = new Connection("RecvConn", 0,
			       itsWH->getDataManager().getInHolder(0), itsTH,
			       false);

      itsWH->getDataManager().setInConnection(0, itsConn);

    }
  }

  void MySocketExample::init() {
    itsWH->basePreprocess();

    if (itsIsSender) {
      ((DH_Example*)itsWH->getDataManager().getInHolder(0))->getBuffer()[0] = makefcomplex(4,3);
    }
  }

  void MySocketExample::run (int nsteps) {

    for (int i = 0; i < nsteps; i++) {
      
      itsWH->baseProcess();
      
    }
  }

  void MySocketExample::run_once() {
    
    itsWH->baseProcess();
    
  }

  void MySocketExample::quit() {
    
    itsWH->basePostprocess();
    
  }


  void MySocketExample::dump() const {
    if (itsIsSender)
    {
      cout << "Sender dump:" << endl;
    }
    else
    {
      cout << "Receiver dump:" << endl;
    }
    itsWH->dump();

  }

} // namespace LOFAR
