#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <iostream>

#define TRACER std::cout << __PRETTY_FUNCTION__ << " (" \
                         << __FILE__ << ":" << __LINE__ << ")\n" 

class X
{
};

class Y
{
};

using namespace LCS::PL;
using namespace std;

template<> void TPersistentObject<X>::doErase() const { TRACER; }
template<> void TPersistentObject<X>::doInsert() const { TRACER; }
template<> void TPersistentObject<X>::doUpdate() const { TRACER; }
template<> void TPersistentObject<X>::init() { TRACER; }
template<> Collection< TPersistentObject<Y> > 
TPersistentObject<Y>::doRetrieve(const Query&, int) 
{ TRACER; return Collection< TPersistentObject<Y> >(); }

int main()
{
  X x;
  Query q;
  PersistenceBroker* b = new PersistenceBroker();

  try {
    cout << "Try to connect to database ...";
    b->connect("dtl_example","postgres","");
  }
  catch (PLException& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

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
