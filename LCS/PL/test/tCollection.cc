#include <PL/Collection.h>
#include <PL/Exception.h>
#include <PL/ObjectId.h>
#include <PL/TPersistentObject.h>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace LCS::PL;

class X
{
public:
  X(int i=0) : _oid(new ObjectId()),_i(i) {}
  void print(ostream& os) const { os << "oid = " << _oid->get() << endl; }
  friend bool operator==(const X& lhs, const X& rhs) 
  { return *(lhs._oid) == *(rhs._oid); }
private:
  boost::shared_ptr<ObjectId> _oid;
  int _i;
};


int main()
{
  cout << endl << "*** Using Collection<X> ***" << endl;
  X x1(1);
  X x2(2);
  Collection<X> cx;
  cout << "Adding two elements ..." << endl;
  cx.add(x1);
  cx.add(x2);
  for(Collection<X>::iterator it=cx.begin(); it!=cx.end(); ++it) {
    it->print(cout);
  }
  cout << "Removing second element ..." << endl;
  cx.remove(x2);
  for(Collection<X>::iterator it=cx.begin(); it!=cx.end(); ++it) {
    it->print(cout);
  }
  cout << "Trying to add first element again ... (this should throw)" << endl;
  try {
    cx.add(x1);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }
  for(Collection<X>::iterator it=cx.begin(); it!=cx.end(); ++it) {
    it->print(cout);
  }

  cout << endl << "*** Using Collection<TPersistentObject<X> > ***" << endl;
  TPersistentObject<X> pox1(x1);
  TPersistentObject<X> pox2(x2);
  Collection<TPersistentObject<X> > cpox;
  cout << "Adding two elements ..." << endl;
  cpox.add(pox1);
  cpox.add(pox2);
  for(Collection<TPersistentObject<X> >::iterator it=cpox.begin(); 
      it!=cpox.end(); ++it) {
    it->value().print(cout);
  }
  cout << "Removing second element ..." << endl;
  cpox.remove(pox2);
  for(Collection<TPersistentObject<X> >::iterator it=cpox.begin(); 
      it!=cpox.end(); ++it) {
    it->value().print(cout);
  }
  cout << "Trying to add first element again ... (this should throw)" << endl;
  try {
    cpox.add(pox1);
  }
  catch (PLException& e) {
    cerr << e << endl;
  }
  for(Collection<TPersistentObject<X> >::iterator it=cpox.begin(); 
      it!=cpox.end(); ++it) {
    it->value().print(cout);
  }

  return 0;
}
