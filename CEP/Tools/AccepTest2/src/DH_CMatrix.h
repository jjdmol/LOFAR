//# DH_CMatrix.h: DataHolder containing a 2D complex matrix
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef DH_CMATRIX_H
#define DH_CMATRIX_H

#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
#define AXISNAMESIZE 20

  /**
     This class represents a 2D matrix of complex values.
  */

  class DH_CMatrix: public DataHolder {
  public:
    typedef LOFAR::dcomplex valueType;
    explicit DH_CMatrix (const string& name = "", 
			 int xsize = 0, 
			 int ysize = 0, 
			 const string& xname = "x", 
			 const string& yname = "y");
    DH_CMatrix(const DH_CMatrix&);
    virtual ~DH_CMatrix();
    virtual DataHolder* clone() const;

    // Allocate room for the actual data
    virtual void preprocess();

    class Axis {
    public:
      inline void setName(const char* name);
      inline char* getName();
      inline void setBegin(float begin);
      inline float getBegin();
      inline void setEnd(float end);
      inline float getEnd();
      friend void dataConvert (DataFormat fmt, DH_CMatrix::Axis* buf, uint nrval);

    private:
      char itsName[AXISNAMESIZE];
      float itsBegin;
      float itsEnd;
    };

    inline Axis& getXaxis();
    inline Axis& getYaxis();
    inline Axis& getAxis(const char* name);
    inline int getXsize();
    inline int getYsize();
    //    inline void setValue(valueType c);
    inline valueType& value(int x, int y);

  private:
    // Forbid assignment.
    DH_CMatrix& operator= (const DH_CMatrix&);

    // Fill the pointers
    virtual void fillDataPointers();

    // description of the axes of the matrix
    Axis* itsXaxis;
    Axis* itsYaxis;
    // the matrix itself
    valueType* itsMatrix;

    // initialization stuff
    // size of the matrix
    int itsXsize;
    int itsYsize;
    // defaultnames of the axes
    string itsXname;
    string itsYname;
  };

  inline void DH_CMatrix::Axis::setName(const char* name) {
    strncpy(itsName, name, AXISNAMESIZE);
  }
  inline char* DH_CMatrix::Axis::getName() {
    return itsName;
  }
  inline void DH_CMatrix::Axis::setBegin(float begin) {
    itsBegin = begin;
  }
  inline float DH_CMatrix::Axis::getBegin() {
    return itsBegin;
  }
  inline void DH_CMatrix::Axis::setEnd(float end) {
    itsEnd = end;
  }
  inline float DH_CMatrix::Axis::getEnd() {
    return itsEnd;
  }

  inline DH_CMatrix::Axis& DH_CMatrix::getXaxis() {
    return *itsXaxis;
  }
  inline DH_CMatrix::Axis& DH_CMatrix::getYaxis() {
    return *itsYaxis;
  }
  inline DH_CMatrix::Axis& DH_CMatrix::getAxis(const char* name) {
    if (strncmp(name, itsXaxis->getName(), AXISNAMESIZE) == 0)
      return *itsXaxis;
    //else if (strncmp(name, itsYaxis, AXISNAMESIZE) == 0)
    //return *itsYaxis;
    else
      // what should we return if no axis matches the given name??
      return *itsYaxis;
  }
  // replaced by value(int, int)
  inline int DH_CMatrix::getXsize() {
    return itsXsize;
  }
  inline int DH_CMatrix::getYsize(){
    return itsYsize;
  }
  inline DH_CMatrix::valueType& DH_CMatrix::value(int x, int y){
    ASSERT(x<itsXsize);//, "x out of bounds in DH_CMatrix::value");
    ASSERT(y<itsYsize);//, "y out of bounds in DH_CMatrix::value");
    // right now a column is a range in memory
    // the only place where parts of the matrix are used at once is in fftw
    // in other places the value method is used
    return itsMatrix[y + itsYsize * x];
  }
} // namespace LOFAR

#endif 
