#include <PL/Collection.h>
#include <PL/ObjectId.h>
#include <PL/TPersistentObject.h>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace LOFAR::PL;

class X
{
public:
  X(int i=0) : _oid(new ObjectId()),_i(i) {}
  void print(ostream& os) const { os << "oid = " << _oid->get() << endl; }
  friend bool operator==(const X& lhs, const X& rhs) 
  { return *lhs._oid == *rhs._oid; }
private:
  boost::shared_ptr<ObjectId> _oid;
  int _i;
};

namespace LOFAR
{
  namespace PL
  {

    template<>
    struct DBRep<X>
    {
      void bindCols(dtl::BoundIOs& cols) {}
    };

    template<> void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const { }
  
    template<> void TPersistentObject<X>::fromDBRep(const DBRep<X>& src) { }

    void TPersistentObject<X>::init() {}

    void TPersistentObject<X>::initAttribMap() {}

  } // namespace PL

} // namespace LOFAR


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
    cout << ">>>" << endl;
    it->print(cout);
    cout << "<<<" << endl;
  }

  cout << "Removing second element ..." << endl;
  cx.remove(x2);
  for(Collection<X>::iterator it=cx.begin(); it!=cx.end(); ++it) {
    cout << ">>>" << endl;
    it->print(cout);
    cout << "<<<" << endl;

  }
  cout << "Trying to add first element again ..." << endl;
  cx.add(x1);
  for(Collection<X>::iterator it=cx.begin(); it!=cx.end(); ++it) {
    cout << ">>>" << endl;
    it->print(cout);
    cout << "<<<" << endl;
  }

  cout << "Removing all elements that match with first element ..." << endl;
  cx.remove(x1);
  for(Collection<X>::iterator it=cx.begin(); it!=cx.end(); ++it) {
    cout << ">>>" << endl;
    it->print(cout);
    cout << "<<<" << endl;
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
    cout << ">>>" << endl;
    it->data().print(cout);
    cout << "<<<" << endl;
  }
  cout << "Removing second element ..." << endl;
  cpox.remove(pox2);
  for(Collection<TPersistentObject<X> >::iterator it=cpox.begin(); 
      it!=cpox.end(); ++it) {
    cout << ">>>" << endl;
    it->data().print(cout);
    cout << "<<<" << endl;
  }
  cout << "Trying to add first element again ... " << endl;
  cpox.add(pox1);
  for(Collection<TPersistentObject<X> >::iterator it=cpox.begin(); 
      it!=cpox.end(); ++it) {
    cout << ">>>" << endl;
    it->data().print(cout);
    cout << "<<<" << endl;
  }
  cout << "Removing all elements that match with first element ..." << endl;
  cpox.remove(pox1);
  for(Collection<TPersistentObject<X> >::iterator it=cpox.begin(); 
      it!=cpox.end(); ++it) {
    cout << ">>>" << endl;
    it->data().print(cout);
    cout << "<<<" << endl;
  }

  return 0;
}
