#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <iostream>

class X
{
};

class Y
{
};

using namespace LCS::PL;
using namespace std;

// template<>
// void TPersistentObject<X>::save(const PersistenceBroker* const) 
// {
//   cout << __PRETTY_FUNCTION__ << endl;
// }

// template<> 
// void TPersistentObject<X>::retrieve(const PersistenceBroker* const)
// {
//   cout << __PRETTY_FUNCTION__ << endl;
// }

int main()
{
  X x;
  Query q;
  PersistenceBroker* b = new PersistenceBroker();

  try {
    cout << "Try to connect to database ..." << endl;
    b->connect("dtl_example","postgres");
  }
  catch (PLException& e) {
    cerr << e << endl;
  }

  TPersistentObject<X> tpox(x);
  try {
    cout << "Trying save() ..." << endl;
    b->save(tpox);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }

  try {
    cout << "Forcing insert() ..." << endl;
    b->save(tpox, PersistenceBroker::INSERT);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }

  try {
    cout << "Forcing update() ..." << endl;
    b->save(tpox, PersistenceBroker::UPDATE);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }

  try {
    cout << "Trying erase() ..." << endl;
    b->erase(tpox);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }

  try {
    cout << "Trying retrieve() ..." << endl;
    b->retrieve<Y>(q);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }

  return 0;
}
