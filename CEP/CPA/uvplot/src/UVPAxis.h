// Copyright notice should go here

#if !defined(UVPAXIS_H)
#define UVPAXIS_H

// $ID$

// Includes
#include <string>



//! Manages the properties of a coordinate axis of a dataset.
/*!  The UVPAxis class is used to scale and shift coordinates. One of
  its applications is the mapping of a value in the real world to a
  value in the pixel or intensity domain. For example for making graphs
  of data.
 */
class UVPAxis
{
 public:

  //! Constructor.
  /*! \param scale may not be 0.0, \param offset may have any value,
    \param type indicates what is plotted along the axis and \param
    unit the units used. */

   UVPAxis(double scale = 1.0, double offset = 0.0, const std::string&
           type="", const std::string& unit="");

  //! \param scale may not be 0.0, \param offset may have any value.
  void        setTransferFunction(double scale,
                                  double offset);


  //! Calculates scale and offset of the transfer function from the
  //! provided world- and axis-ranges.

  /*! calcTransferFunction calculates itsScale and itsOffset fromn the
    world and axis ranges that are provided by the caller.  \param
    axisMin and \param axisMax may be pixel coordinates on a
    screen. axisMin may not be equal to axisMax. \param worldMin may
    not be equal to \param worldMax. */
  void        calcTransferFunction(double worldMin, 
                                   double worldMax,
                                   double axisMin,
                                   double axisMax);

  //! axis = (world - itsOffset)/itsScale
  inline double      worldToAxis(double world) const;

  //! world = itsOffset + itsScale*axis;
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
