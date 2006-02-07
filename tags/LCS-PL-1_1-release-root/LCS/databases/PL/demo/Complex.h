#ifndef LOFAR_PL_DEMO_COMPLEX_H
#define LOFAR_PL_DEMO_COMPLEX_H

#include <complex>
#include <iostream>
#include <PL/PLfwd.h>

class Complex
{
public:
  Complex() : itsValue(0.0, 0.0) {}
  Complex(double re, double im=0.0) : itsValue(re, im) {}
  friend std::ostream& operator<<(std::ostream& os, const Complex& c);
  friend std::istream& operator>>(std::istream& is, Complex& c);
private:
  friend class LOFAR::PL::TPersistentObject<Complex>;
  std::complex<double> itsValue;
};

inline std::ostream& operator<<(std::ostream& os, const Complex& c)
{
  return os << c.itsValue; 
}

inline std::istream& operator>>(std::istream& is, Complex& c)
{
  return is >> c.itsValue;
}

#endif
