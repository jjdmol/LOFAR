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
    broker.connect(getUserName(),"postgres");

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

    Query::Expr expr;

    // This makes for an interesting case, because it doesn't work :-p.
    // The problem lies with the fact that class C inherits from class A,
    // which in turn has a field itsB. There is, however, no way to tell the
    // TPO<C> that it should also inquire its "parent" TPO<A> to look for a
    // field itsB. Hence, the lookup in the attribute map fails.
    try {
      expr = attrib<C>("itsB.itsShort") == 327;
    } catch (QueryError& e){
      cerr << e << endl;
    }

    // The workaround, to use attrib<A>(), does not produce the right query
    // result, because the constraint (C.OBJID=A.OWNER) will not be generated!
    expr = attrib<A>("itsB.itsShort") == 327;

    // Ah, this can be done. The attrib() method is not completely useles :-p.
    // We have to make sure, though, that the template parameter in the call
    // to attrib() is the same as that in the call to
    // broker.retrieve(). Hence, we're currently limited to composing the
    // query for fields of class C only.
    expr = attrib<C>("itsString") == "C4Y2";
    cout << "Retrieve collection of tpoc using query: " << expr << endl;

    Collection< TPersistentObject<C> > ctpoc;
    Collection< TPersistentObject<C> >::const_iterator iter;
    ctpoc = broker.retrieve<C>(QueryObject(expr));
    cout << "Found " << ctpoc.size() << " matches ..." << endl;
    for(iter = ctpoc.begin(); iter != ctpoc.end(); ++iter) {
      cout << "Press <Enter> to continue" << endl;
      cin.get();
      cout << iter->metaData() << endl;
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
