// Copyright notice should go here

#if !defined(UVPAXIS_H)
#define UVPAXIS_H

// $ID$

// Includes
#include <string>



// Manages the properties of a coordinate axis of a dataset.
class UVPAxis
{
 public:

  // Constructor. scale must be > 0.0, offset may be any value, type
  // indicates what is plotted along the axis and unit the units
  // used.
              UVPAxis(double             scale  = 1.0,
                      double             offset = 0.0,
                      const std::string& type="",
                      const std::string& unit="");

  // scale may not be 0.0
  void        setTransferFunction(double scale,
                                  double offset);

  // axisMin and axisMax may be pixel coordinates on a screen. This
  // method calculates itsScale and itsOffset from the provided
  // ranges.
  void        calcTransferFunction(double worldMin, 
                                   double worldMax,
                                   double axisMin,
                                   double axisMax);

  // axis = (world - itsOffset)/itsScale
  inline double      worldToAxis(double world) const;

  // world = itsOffset + itsScale*axis;
  inline double      axisToWorld(double axis) const;

  
  double      getScale() const;
  double      getOffset() const;
  std::string getType() const;
  std::string getUnit() const;



 private:
  
  double      itsScale;
  double      itsOffset;

  std::string itsType;
  std::string itsUnit;
};





//====================>>>  UVPAxis::worldToAxis  <<<====================

inline double UVPAxis::worldToAxis(double world) const
{
#if(DEBUG_MODE)
  assert(itsScale != 0);
#endif

  return (world - itsOffset)/itsScale;
}






//====================>>>  UVPAxis::axisToWorld  <<<====================

inline double UVPAxis::axisToWorld(double axis) const
{
  return itsOffset + axis*itsScale;
}

#endif // UVPAXIS_H
