#include "A.h"
#include "PO_A.h"
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <PL/Attrib.h>
#include <Common/Exception.h>
#include <complex>
#include <iostream>
#include <pwd.h>

using namespace std;
using namespace LOFAR::PL;
using namespace LOFAR;

int main()
{
  try {

    ObjectId oid;
    PersistenceBroker broker;

    A a;
    A a1(42, 3.14, "Hello", complex<double>(2.818, -2.818), 
	 B(false, -14, -1.7320508, "Bubbles"));
    A a2(84, 6.28, "Goodbye", complex<double>(5.636, -5.636),
	 B(true, 327, 1.4142135, "Bjorn again"));
    
    TPersistentObject<A> tpoa(a);
    TPersistentObject<A> tpoa1(a1);
    TPersistentObject<A> tpoa2;

    // Connect to the database
    broker.connect("test","postgres");

    // Should call insert(), saving data in a
    broker.save(tpoa1); 
    cout << "Saved tpoa1 <-- tpoa1 = " 
         << tpoa1.metaData() << tpoa1.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call insert(), saving data in a
    broker.save(tpoa); 
    cout << "Saved tpoa <-- tpoa = " 
         << tpoa.metaData() << tpoa.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call update(), saving data in a2
    tpoa.data() = a2;
    broker.save(tpoa);
    cout << "Updated tpoa <-- tpoa = " 
         << tpoa.metaData() << tpoa.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a1
    oid.set(tpoa1.metaData().oid()->get());
//     tpoa2 = broker.retrieve<A>(oid);
    tpoa2.retrieve(oid);
    cout << "Retrieved tpoa1 --> tpoa2 = " 
         << tpoa2.metaData() << tpoa2.data() << endl;

    cout << "Press <Enter> to continue";
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a2
    oid.set(tpoa.metaData().oid()->get());
//     tpoa2 = broker.retrieve<A>(oid);
    tpoa2.retrieve(oid);
    cout << "Retrieved tpoa --> tpoa2 = " 
         << tpoa2.metaData() << tpoa2.data() << endl;
    
    cout << "Press <Enter> to continue";
    cin.get();

    // Should erase the database entry for a2
    cout << "Erasing a in tpoa2.data() -- tpoa2 = "
         << tpoa2.metaData() << tpoa2.data() << endl;
    tpoa2.erase();

    cout << "Press <Enter> to continue";
    cin.get();

    QueryObject q(attrib<A>("itsInt") == 42);
    cout << "Retrieve collection of tpoa using query: " << q.getSql() << endl;
    Collection< TPersistentObject<A> > ctpoa;
    ctpoa = broker.retrieve<A>(q);
    cout << "Found " << ctpoa.size() << " matches ..." << endl;
    Collection< TPersistentObject<A> >::const_iterator iter;
    for(iter = ctpoa.begin(); iter != ctpoa.end(); ++iter) {
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
