#include "LCS_base.h"
#include "C.h"
#include "PO_C.h"
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <iostream>

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
    broker.connect("dtl_example","postgres");

    DBConnection::GetDefaultConnection().SetAutoCommit(true);

    cout << "DBConnection::GetDefaultConnection().GetAutoCommit() = "
	 <<  DBConnection::GetDefaultConnection().GetAutoCommit() << endl;

    // Should call insert(), saving data in c
    cout << "Saving tpoc1 -- tpoc1.data() = " << tpoc1.data() << endl;
    broker.save(tpoc1); 
//     cout << tpoc1.metaData() << endl;

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call insert(), saving data in a
    cout << "Saving tpoc -- tpoc.data() = " << tpoc.data() << endl;
    broker.save(tpoc); 
//     cout << tpoc.metaData() << endl;

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call update(), saving data in a2
    tpoc.data() = c2;
    cout << "Saving tpoc1 -- tpoc.data() = " << tpoc.data() << endl;
    broker.save(tpoc);
//     cout << tpoc.metaData() << endl;
    
    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a1
    oid.set(tpoc1.metaData().oid()->get());
    tpoc2 = broker.retrieve<C>(oid);
//     tpoc2.retrieve(oid);
    cout << "Retrieved tpoc1 -- tpoc1.data() = " << tpoc2.data() << endl;
//          << tpoc2.metaData() << endl;

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a2
    oid.set(tpoc.metaData().oid()->get());
    tpoc2 = broker.retrieve<C>(oid);
//     tpoc2.retrieve(oid);
    cout << "Retrieved tpoc -- tpoc.data() = " << tpoc2.data() << endl;
//          << tpoc2.metaData() << endl;
    
    cout << "Press <Enter> to continue" << endl;
    cin.get();

    cout << "Retrieve collection of tpoc using query" << endl;
    Collection< TPersistentObject<C> > ctpoc;
    Collection< TPersistentObject<C> >::const_iterator iter;
    ctpoc = broker.retrieve<C>(Query("WHERE ItsString='C4Y2';"));
    cout << "Found " << ctpoc.size() << " matches ..." << endl;
    for(iter = ctpoc.begin(); iter != ctpoc.end(); ++iter) {
      cout << "Press <Enter> to continue" << endl;
      cin.get();
      cout << iter->data() << endl;
    }

  }
  catch (PLException& e) {
    cerr << e << endl;
    return 1;
  }
  catch (exception& e) {
    cerr << "Caught std::exception: " << e.what() << endl;
    return 1;
  }
  catch (...) {
    cerr << "Caught unknown exception" << endl;
    return 1;
  }
  return 0;
}
