#include "A.h"
#include "PO_A.h"
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <Common/Exception.h>
#include <complex>
#include <iostream>
#include <pwd.h>

using namespace std;
using namespace LOFAR::PL;
using namespace LOFAR;

string getUserName()
{
  passwd* aPwd;
  if ((aPwd = getpwuid(getuid())) == 0)
    return "";
  else
    return aPwd->pw_name;
}

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
    broker.connect(getUserName(),"postgres");

    DBConnection::GetDefaultConnection().SetAutoCommit(true);

    cout << "DBConnection::GetDefaultConnection().GetAutoCommit() = "
	 <<  DBConnection::GetDefaultConnection().GetAutoCommit() << endl;

    // Should call insert(), saving data in a
    cout << "Saving tpoa1 -- tpoa1.data() = " << tpoa1.data() << endl;
    broker.save(tpoa1); 

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call insert(), saving data in a
    cout << "Saving tpoa -- tpoa.data() = " << tpoa.data() << endl;
    broker.save(tpoa); 

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call update(), saving data in a2
    tpoa.data() = a2;
    cout << "Saving tpoa1 -- tpoa.data() = " << tpoa.data() << endl;
    broker.save(tpoa);

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a1
    oid.set(tpoa1.metaData().oid()->get());
    tpoa2 = broker.retrieve<A>(oid);
//     tpoa2.retrieve(oid);
    cout << "Retrieved tpoa1 -- tpoa1.data() = " << tpoa2.data() << endl;

    cout << "Press <Enter> to continue" << endl;
    cin.get();

    // Should call retrieve(ObjectId&), returning a TPO that contains a2
    oid.set(tpoa.metaData().oid()->get());
    tpoa2 = broker.retrieve<A>(oid);
//     tpoa2.retrieve(oid);
    cout << "Retrieved tpoa -- tpoa.data() = " << tpoa2.data() << endl;
    
    cout << "Press <Enter> to continue" << endl;
    cin.get();

    cout << "Retrieve collection of tpoa using query" << endl;
    Collection< TPersistentObject<A> > ctpoa;
    Collection< TPersistentObject<A> >::const_iterator iter;
    ctpoa = broker.retrieve<A>(Query("WHERE ITSINT=42;"));
    cout << "Found " << ctpoa.size() << " matches ..." << endl;
    for(iter = ctpoa.begin(); iter != ctpoa.end(); ++iter) {
      cout << "Press <Enter> to continue" << endl;
      cin.get();
      cout << iter->data() << endl;
    }

  }
  catch (Exception& e) {
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
