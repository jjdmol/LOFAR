#include "myComplex.h"

main () {
  myComplex8 a(1,2);
  myComplex8 b(3,4);

  myComplex16 c;
  c.mult(a,b);
  cout << a << " * " << b << " = " << c << endl;
  
  myComplex32 c32;

  c32.mac(a,b);
  cout << "once mac : " <<  c32 << endl;
  c32.mac(a,b);
  cout << "twice mac : " << c32 << endl;

  c.cmult(a,b);
  cout << a << " * " << b << "*" << " = " << c << endl;

  c32 = 0;
  c32.cmac(a,b);
  cout << "once cmac : " <<  c32 << endl;
  c32.cmac(a,b);
  cout << "twice cmac : " << c32 << endl;

}
