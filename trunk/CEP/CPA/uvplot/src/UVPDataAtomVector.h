// Copyright notice should go here

#if !defined(UVPDATAATOMVECTOR_H)
#define UVPDATAATOMVECTOR_H

// $Id$

#include <uvplot/UVPDataAtom.h>

#include <vector>


//! A list of UVPDataAtom objects.
/*!It maintains min/max values for both the real and imaginary parts
  of the visibilities. Use the "add" method instead of the "push_back"
  method. A UVPDataAtomVector does not destruct the objects it points
  to. It is supposed to live shorter than the UVPDataAtom objects that
  it points to. */

class UVPDataAtomVector:public std::vector<const UVPDataAtom *>
{
public:
   UVPDataAtomVector();
  ~UVPDataAtomVector();
  
  double minRe() const;
  double maxRe() const;
  double minIm() const;
  double maxIm() const;

  double min() const;
  double max() const;

  //!If honourFlags == true, ignore flagged data in min/max computation
  void   add(const UVPDataAtom *atom,
             bool               honourFlags=false);
  
protected:
private:
  
  double itsMinRe;
  double itsMaxRe;
  double itsMinIm;
  double itsMaxIm;
  
  double itsMin;
  double itsMax;
};


#endif
