#ifndef MYCOMPLEX_H
#define MYCOMPLEX_H

#define  COMPMULT real  = a.real*b.real - a.imag*b.imag;   imag  = a.imag*b.real + a.real*b.imag;
#define CCOMPMULT real  = a.real*b.real + a.imag*b.imag;   imag  = a.imag*b.real - a.real*b.imag;
#define  COMPMAC  real += a.real*b.real - a.imag*b.imag;   imag += a.imag*b.real + a.real*b.imag;
#define CCOMPMAC  real += a.real*b.real + a.imag*b.imag;   imag += a.imag*b.real - a.real*b.imag;

#include <iostream>

class myComplex8 {
 public:
  myComplex8() {real=0; imag=0;}
  myComplex8(int r, int i) {real = r; imag = i;}
  ~myComplex8() {};

  void  mult(const myComplex8 &a, const myComplex8 &b);
  void cmult(const myComplex8 &a, const myComplex8 &b);
  
  friend std::ostream &operator<<(std::ostream &s, const myComplex8 &a) {
    return s << "(" << (int)a.real << "," << (int)a.imag << ")";
  }

  myComplex8& operator= (const myComplex8& a) {real = a.real; imag= a.imag; return *this;}
  myComplex8& operator= (const int& a) {real = a; imag= 0; return *this;}


  char real;
  char imag;
};


class myComplex16 {
 public:
  
  myComplex16() {real=0; imag=0;}
  myComplex16(int r, int i) {real = r && 0xFFFF0000; imag = i && 0xFFFF0000;}
  ~myComplex16() {};

  void  mult(const myComplex8 &a, const myComplex8 &b);
  void cmult(const myComplex8 &a, const myComplex8 &b);

  friend std::ostream &operator<<(std::ostream &s, const myComplex16 &a) {
    return s << "(" << a.real << "," << a.imag << ")";
  }
  myComplex16& operator= (const myComplex16& a) {real = a.real; imag= a.imag; return *this;}
  myComplex16& operator= (const int& a) {real = a; imag= 0; return *this;}

  short real;
  short imag;

};


class myComplex32 {
 public:

  myComplex32() {real=0; imag=0;}
  myComplex32(int r, int i) {real = r; imag = i;}
  ~myComplex32() {};

  void  mac(const myComplex8 &a, const myComplex8 &b);
  void cmac(const myComplex8 &a, const myComplex8 &b);

  friend std::ostream &operator<<(std::ostream &s, const myComplex32 &a) {
    return s << "(" << a.real << "," << a.imag << ")";
  }
  myComplex32& operator= (const myComplex32& a) {real = a.real; imag= a.imag; return *this;}
  myComplex32& operator= (const int& a) {real = a; imag= 0; return *this;}

  int real;
  int imag;

};


inline void myComplex8::cmult (const myComplex8 &a, const myComplex8 &b) { CCOMPMULT } 
inline void myComplex8::mult  (const myComplex8 &a, const myComplex8 &b) {  COMPMULT }
inline void myComplex16::cmult(const myComplex8 &a, const myComplex8 &b) { CCOMPMULT }
inline void myComplex16::mult (const myComplex8 &a, const myComplex8 &b) {  COMPMULT }
inline void myComplex32::cmac (const myComplex8 &a, const myComplex8 &b) { CCOMPMAC  }
inline void myComplex32::mac  (const myComplex8 &a, const myComplex8 &b) {  COMPMAC  }


#endif
