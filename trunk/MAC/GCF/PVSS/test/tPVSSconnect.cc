//
//  tPVSSconnect.cc: Test program to test the machanism of connecting to (remote) databases.
//
//  Copyright (C) 2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <HotLinkWaitForAnswer.hxx>   
#include <Resources.hxx>
#include <Manager.hxx>
#include <StartDpInitSysMsg.hxx>

using namespace LOFAR;
using namespace LOFAR::GCF::PVSS;

// ---------- MyHotlink ----------
class MyHotLink : public HotLinkWaitForAnswer
{
public:
    MyHotLink ();
    virtual ~MyHotLink () {};
 
    void hotLinkCallBack (DpMsgAnswer& answer);
    void hotLinkCallBack (DpHLGroup& group);
};

MyHotLink::MyHotLink() :
    HotLinkWaitForAnswer()
{
}

void MyHotLink::hotLinkCallBack(DpMsgAnswer& /*answer*/)
{
	cout << "hotLinkCallBack(Answer)" << endl;
}


void MyHotLink::hotLinkCallBack(DpHLGroup& /*group*/)
{
    cout << "hotLinkCallBack(group)" << endl;
}


// ---------- MyResources ----------
class MyResources : public Resources
{ 
public:   
    // These functions initializes the manager
    static void init (int &argc, char *argv[]);  
                    
    // Read the config section
    static PVSSboolean readSection ();
};

void  MyResources::init(int &argc, char *argv[])
{
  Resources::setManNum(0);
  begin(argc, argv);

  while ( readSection() || generalSection() )
    ;
  end(argc, argv);
}

PVSSboolean MyResources::readSection()
{
	if (!isSection("GCF")) {
		return (PVSS_FALSE);
	}

	getNextEntry();
	while (cfgState != CFG_SECT_START && cfgState != CFG_EOF) {
		if (!readGeneralKeyWords()) {
			cout << "Illegal Keyword: " << keyWord << endl;
			cfgError = PVSS_TRUE;
		}
		getNextEntry();
	}
	return (cfgState != CFG_EOF);
}

// ---------- MyManager ----------
class MyManager : public Manager
{
public:
//    friend class GSASCADAHandler;
    MyManager();
    virtual ~MyManager() {};
};

MyManager::MyManager() :
  Manager(ManagerIdentifier(API_MAN, Resources::getManNum()))
{
  // First connect to Data manager.
  // We want Typecontainer and Identification so we can resolve names
  // This call succeeds or the manager will exit
  cout << "connectToData" << endl;
  connectToData(StartDpInitSysMsg::TYPE_CONTAINER | StartDpInitSysMsg::DP_IDENTIFICATION);

  // While we are in STATE_INIT  we are initialized by the Data manager
  long sec, usec;
  while (getManagerState() == STATE_INIT) {
    sec = 1;
    usec = 0;
    dispatch(sec, usec);
  }

  // We are now in STATE_ADJUST and can connect to Event manager
  // This call will succeed or the manager will exit
  cout << "connectToEvent" << endl;
  connectToEvent();

  if (getManagerState() == STATE_RUNNING) {
    cout << "Application connected to PVSS system" << endl;
  }
  // We are now hopefully in STATE_RUNNING. 
}


int main(int argc, char* argv[])
{
	argv[argc++] = "-event";
	argv[argc++] = "cs001t.lofartest.nl";
	argv[argc++] = "-data";
	argv[argc++] = "cs001t.lofartest.nl";
//	argv[argc++] = "-currentproj";
	argv[argc++] = "-proj";
	argv[argc++]  ="CS001T";
	argv[argc++] = "-log";
	argv[argc++] = "+stderr";
	
	cout << "constructing hotlink" << endl;
	MyHotLink	someHotLink;

	cout << "initializing resources" << endl;
	MyResources::init(argc, argv);

	cout << "construction Manager" << endl;
	MyManager	someManager;

  	cout << "project name: " << PVSSinfo::getProjectName() << endl;

	sleep (5);

//	...

	return 0;
}
