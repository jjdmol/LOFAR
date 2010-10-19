//#  tA.cc: Test program that stores/retrieves data to/from a database
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include "A.h"
#include "PO_A.h"
#include <PL/PersistenceBroker.h>
#include <PL/QueryObject.h>
#include <PL/Attrib.h>
#include <Common/Exception.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>

using namespace LOFAR::PL;
using namespace LOFAR;

int main(int argc, const char* argv[])
{
  try {

    ObjectId oid;
    PersistenceBroker broker;
    int ret;

    A a;
    A a1(42, 3.14, "Hello", makedcomplex(2.818, -2.818), 
	 B(false, -14, -1.7320508, "Bubbles"));
    A a2(84, 6.28, "Goodbye", makedcomplex(5.636, -5.636),
	 B(true, 327, 1.4142135, "Bjorn again"));
    
    TPersistentObject<A> tpoa(a);
    TPersistentObject<A> tpoa1(a1);
    TPersistentObject<A> tpoa2;

    // Initialize the logger
    INIT_LOGGER(argv[0]);

    // Connect to the database
    broker.connect("test","postgres");

    // Should call insert(), saving a1 in database
    broker.save(tpoa1); 
    cout << "Saved tpoa1 <-- tpoa1 : " << endl;
    cout << ">>>" << endl; 
    cout << tpoa1.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoa1.data() << endl;

    // Systemcall to extract a1 from database outside this program
    // a1 should contain the a1 initial data mentioned above
    cout << endl << "Retrieving tpoa1 (a1) from database: " << endl;
    oid.set(tpoa1.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tA.in_sqld " << oid.get();
    
      ret = system(oss.str().c_str());

      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should call insert(), saving a in the database
    broker.save(tpoa); 
    cout << "Saved tpoa <-- tpoa : " << endl;
    cout << ">>>" << endl; 
    cout << tpoa.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoa.data() << endl;

    // Systemcall to extract a from database outside this program
    // a should be empty
    cout << endl << "Retrieving tpoa (empty) from database: " << endl;
    oid.set(tpoa.metaData().oid()->get());

    {
      stringstream oss;
      oss << "./tA.in_sqld " << oid.get();
      
      ret = system(oss.str().c_str());
      
      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }

    // fill a with a2 data
    tpoa.data() = a2;
    // Should call update(), saving a2 data in a
    broker.save(tpoa);
    cout << "Updated tpoa <-- tpoa : " << endl;
    cout << ">>>" << endl; 
    cout << tpoa.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoa.data() << endl;

    // Systemcall to extract a from database outside this program
    // a should contain a2 initial data mentioned above
    cout << endl << "Retrieving tpoa (a2) from database: " << endl;
    oid.set(tpoa.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tA.in_sqld " << oid.get();
      
      ret = system(oss.str().c_str());
      
      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should call retrieve(ObjectId&), returning a TPO that contains a1
    oid.set(tpoa1.metaData().oid()->get());
    tpoa2.retrieve(oid);

    cout << "Retrieved tpoa1 --> tpoa2 : " << endl; 
    cout << ">>>" << endl; 
    cout << tpoa2.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoa2.data() << endl;

    // Systemcall to extract a2 from database outside this program
    // a2 should contain a1 initial data mentioned above
    cout << endl << "Retrieving tpoa2 (a1) from database: " << endl;
    oid.set(tpoa2.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tA.in_sqld " << oid.get();
      
      ret = system(oss.str().c_str());
      
      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should call retrieve(ObjectId&), returning a TPO that contains a
    oid.set(tpoa.metaData().oid()->get());

    tpoa2.retrieve(oid);
    cout << "Retrieved tpoa --> tpoa2 : " << endl; 
    cout << ">>>" << endl; 
    cout << tpoa2.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoa2.data() << endl;
    
    // Systemcall to extract a from database outside this program
    // a should contain a2 initial data mentioned above
    cout << endl << "Retrieving tpoa2 (a2) from database: " << endl;
    oid.set(tpoa2.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tA.in_sqld " << oid.get();
      
      ret = system(oss.str().c_str());
      
      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should erase the database entry for a2
    cout << "Erasing a in tpoa2.data() -- tpoa2 : " << endl;
    cout << ">>>" << endl; 
    cout << tpoa2.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoa2.data() << endl;

    tpoa2.erase();

    // Systemcall to extract a2 from database outside this program
    // a2 should be empty
    cout << endl << "Retrieving tpoa2 (empty) from database: " << endl;
    oid.set(tpoa2.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tA.in_sqld " << oid.get();
      
      ret = system(oss.str().c_str());
      
      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // retrieve  collection from database where itsInt == 42
    QueryObject q(attrib<A>("itsInt") == 42);
    cout << "Retrieve collection of tpoa using query: " << q.getSql() << endl;
    Collection< TPersistentObject<A> > ctpoa;
    ctpoa = broker.retrieve<A>(q);
    cout << "Found " << ctpoa.size() << " matches ..." << endl;
    Collection< TPersistentObject<A> >::const_iterator iter;
    for(iter = ctpoa.begin(); iter != ctpoa.end(); ++iter) {

      cout << ">>>" << endl; 
      cout << iter->metaData() << endl;
      cout << "<<<" << endl;
      cout << iter->data() << endl;

      // Systemcall to extract iter from database outside this program
      // a should show all entries that have itsInt=42 (
      cout << endl << "Retrieving all from database where itsInt=42 (1) : " << endl;
      oid.set(iter->metaData().oid()->get());
      {
	stringstream oss;
	oss << "./tA.in_sqld " << oid.get();
    
	ret = system(oss.str().c_str());
	
	cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
      }
    }

  }
  catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }
  catch (std::exception& e) {
    cerr << "Caught std::exception: " << e.what() << endl;
    return 1;
  }
  catch (...) {
    cerr << "Caught unknown exception" << endl;
    return 1;
  }
  return 0;
}
