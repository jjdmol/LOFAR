#include "C.h"
#include "PO_C.h"
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <PL/Attrib.h>
#include <iostream>
#include <pwd.h>

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
  
  try {

    ObjectId oid;
    PersistenceBroker broker;

    TPersistentObject<C> tpoc(c);
    TPersistentObject<C> tpoc1(c1);
    TPersistentObject<C> tpoc2;
    
    // Connect to the database
    broker.connect("test","postgres");

    // Should call insert(), saving data in c
    broker.save(tpoc1); 
    cout << "Saved tpoc1 <-- tpoc1 = " 
         << tpoc1.metaData() << tpoc1.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call insert(), saving data in c
    broker.save(tpoc); 
    cout << "Saved tpoc <-- tpoc = " 
         << tpoc.metaData() << tpoc.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call update(), saving data in c2
    tpoc.data() = c2;
    broker.save(tpoc);
    cout << "Updated tpoc <-- tpoc = " 
         << tpoc.metaData() << tpoc.data() << endl;
    
    cout << "Press <Enter> to continue";
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a1
    oid.set(tpoc1.metaData().oid()->get());
    tpoc2.retrieve(oid);
    cout << "Retrieved tpoc1 --> tpoc2 = " 
         << tpoc2.metaData() << tpoc2.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a2
    oid.set(tpoc.metaData().oid()->get());
    tpoc2.retrieve(oid);
    cout << "Retrieved tpoc --> tpoc2 = " 
         << tpoc2.metaData() << tpoc2.data() << endl;
    
    cout << "Press <Enter> to continue";
    cin.get();

    QueryObject q(attrib<C>("itsString") == "C4Y2");
    cout << "Retrieve collection of tpoc using query: " << q.getSql() << endl;
    Collection< TPersistentObject<C> > ctpoc;
    ctpoc = broker.retrieve<C>(q);
    cout << "Found " << ctpoc.size() << " matches ..." << endl;
    Collection< TPersistentObject<C> >::const_iterator iter;
    for(iter = ctpoc.begin(); iter != ctpoc.end(); ++iter) {
      cout << "Press <Enter> to continue";
      cin.get();
      cout << iter->metaData() << iter->data() << endl;
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
