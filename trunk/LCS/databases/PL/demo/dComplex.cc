#include "PO_Complex.h"
#include <PL/PersistenceBroker.h>
#include <PL/TPersistentObject.h>
#include <iostream>

using namespace LOFAR::PL;
using namespace std;

int main()
{
  try {
    PersistenceBroker b;
    b.connect("test", "postgres");
    while (cin) {
      Complex c;
      cout << "Enter a complex number (CTRL-D to skip) : ";
      cin >> c;
      if (cin) b.save(TPersistentObject<Complex>(c));
    }
    typedef Collection< TPersistentObject<Complex> > CPOCmplx;
    CPOCmplx cc = b.retrieve<Complex>(QueryObject());
    cout << endl << "Retrieved " << cc.size() << " objects." << endl;
    CPOCmplx::const_iterator cit;
    for(cit = cc.begin(); cit != cc.end(); ++cit) {
      cout << cit->data() << endl;
    }
  } catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return 0;
}
