#include "C.h"
#include "PO_C.h"
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <PL/Attrib.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace LOFAR::PL;

int main()
{

  A a1(42, 3.14, "Hello", complex<double>(2.818, -2.818), 
       B(false, -14, -1.7320508, "Bubbles"));
  A a2(84, 6.28, "Goodbye", complex<double>(5.636, -5.636),
       B(true, 327, 1.4142135, "Bjorn again"));
  blob b1((dtl::BYTE*)"ABCDEFG");
  blob b2((dtl::BYTE*)"abcdefg");
  C c;
  C c1(a1, b1, "CU soon");
  C c2(a2, b2, "C4Y2");

  int ret;
  
  try {

    ObjectId oid;
    PersistenceBroker broker;

    TPersistentObject<C> tpoc(c);
    TPersistentObject<C> tpoc1(c1);
    TPersistentObject<C> tpoc2;
    
    // Connect to the database
    broker.connect("test","postgres");

    // Should call insert(), saving data in c1
    broker.save(tpoc1); 
    cout << "Saved tpoc1 <-- tpoc1 : " << endl;
    cout << ">>>" << endl; 
    cout << tpoc1.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoc1.data() << endl;

    // Systemcall to extract c1 from database outside this program
    // c1 should contain the c1 initial data mentioned above
    cout << endl << "Retrieving tpoc1 (c1) from database: " << endl;
    oid.set(tpoc1.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tC.in_sqld " << oid.get();

      ret = system(oss.str().c_str());

      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }

    // Should call insert(), saving c in database
    broker.save(tpoc); 
    cout << "Saved tpoc <-- tpoc : " << endl;
    cout << ">>>" << endl; 
    cout << tpoc.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoc.data() << endl;

    // Systemcall to extract c from database outside this program
    // c should be empty
    cout << endl << "Retrieving tpoc (empty) from database: " << endl;
    oid.set(tpoc.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tC.in_sqld " << oid.get();

      ret = system(oss.str().c_str());

      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should call update(), saving c2 data in c
    tpoc.data() = c2;
    broker.save(tpoc);
    cout << "Saved tpoc <-- tpoc : " << endl;
    cout << ">>>" << endl; 
    cout << tpoc.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoc.data() << endl;

    // Systemcall to extract c from database outside this program
    // c should contain c2 data
    cout << endl << "Retrieving tpoc (c2) from database: " << endl;
    oid.set(tpoc.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tC.in_sqld " << oid.get();

      ret = system(oss.str().c_str());

      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should call retrieve(ObjectId&), returning a TPO that contains c1
    oid.set(tpoc1.metaData().oid()->get());
    tpoc2.retrieve(oid);
    cout << "Retrieved tpoc1 --> tpoc2 : " << endl;
    cout << ">>>" << endl; 
    cout << tpoc2.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoc2.data() << endl;

    // Systemcall to extract c2 from database outside this program
    // c2 should contain c1 data
    cout << endl << "Retrieving tpoc2 (c1) from database: " << endl;
    oid.set(tpoc2.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tC.in_sqld " << oid.get();

      ret = system(oss.str().c_str());

      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }


    // Should call retrieve(ObjectId&), returning a TPO that contains c2
    oid.set(tpoc.metaData().oid()->get());
    tpoc2.retrieve(oid);
    cout << "Retrieved tpoc --> tpoc2 : " << endl;
    cout << ">>>" << endl; 
    cout << tpoc2.metaData() << endl;
    cout << "<<<" << endl;
    cout << tpoc2.data() << endl;

    // Systemcall to extract c2 from database outside this program
    // c2 should contain c
    cout << endl << "Retrieving tpoc2 (c) from database: " << endl;
    oid.set(tpoc2.metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tC.in_sqld " << oid.get();

      ret = system(oss.str().c_str());

      cout << "System call >>>" << oss.str() << "<<< returned : " << ret << endl;
    }

    QueryObject q(attrib<C>("itsString") == "C4Y2");
    cout << "Retrieve collection of tpoc using query: " << q.getSql() << endl;
    Collection< TPersistentObject<C> > ctpoc;
    ctpoc = broker.retrieve<C>(q);
    cout << "Found " << ctpoc.size() << " matches ..." << endl;
    Collection< TPersistentObject<C> >::const_iterator iter;
    for(iter = ctpoc.begin(); iter != ctpoc.end(); ++iter) {
      cout << ">>>" << endl; 
      cout << iter->metaData() << endl;
      cout << "<<<" << endl;
      cout << iter->data() << endl;

    // Systemcall to extract iter from database outside this program
    // c should be empty
    cout << endl << "Retrieving all from database where itsString=C4Y2 : " << endl;
    oid.set(iter->metaData().oid()->get());
    {
      stringstream oss;
      oss << "./tC.in_sqld " << oid.get();

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
