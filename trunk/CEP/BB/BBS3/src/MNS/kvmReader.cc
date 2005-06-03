//# kvmReader.cc: read values from a KeyValueMap file
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

#include <lofar_config.h>
#include <iostream>
#include <fstream>

#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>
#include <Common/KeyParser.h>
#include <string>

using namespace LOFAR;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

string dbHost, dbName, dbType, dbUser, tableName;

void printValue(const KeyValueMap& kvm, char* keylist[], int noKeysLeft) {
  if (!kvm.isDefined(keylist[0])) {
    //    cerr<<"value not defined"<<endl;
    //    cout<<"default: ";
    if (noKeysLeft > 0) {
      cout<<keylist[noKeysLeft]<<endl; // this key wasn't found, so return default value
    } // if there are no values left return nothing
  } else {
    KeyValueMap nkvm = kvm;
    KeyValue value = nkvm[keylist[0]];
    switch (value.dataType())
      {
      case KeyValue::DTValueMap:
	//	cerr<<"value is a map"<<endl;
	if (noKeysLeft == 0) {
	  cout<<value.getValueMap()<<endl;;
	} else {
	  printValue(value.getValueMap(), keylist+1, noKeysLeft-1);
	}
	break;
      default:
	//	cerr<<"value is:"<<endl;
	cout<<value<<endl;
      }
  };
};

int main(int argc, char* argv[])
{
  if (argc < 3) {
    cerr<<"Usage: kvmReader <kvmFile> <kvmKey> [<kvmKey>] ... [<defaultValue>]"<<endl;
  } else {
    try {
      ifstream ifstr(argv[1]);
      string keyv;
      if (ifstr) {
	string str;
	while (getline(ifstr, str)) {
	  if (str.size() > 0) {
	    // Remove possible comments.
	    string::size_type pos = str.find("//");
	    if (pos == string::npos) {
	      keyv += str;
	    } else if (pos > 0) {
	      keyv += str.substr(0,pos);
	    }
	  }
	}
      } else { //ifstr cannot be opened, maybe it is a kvm string, this doesn't work if the string has spaces
	keyv=argv[1];
      }
      
      //      cout<<"parsing "<<endl;
      KeyValueMap kvm = KeyParser::parse (keyv);
      //      cout<<"printing"<<endl;
      printValue(kvm, argv+2, argc-3);
    } catch (Exception &e) {
      cerr<<"Exception: "<<e.what()<<endl;
    }
  }
  return 0;
}
