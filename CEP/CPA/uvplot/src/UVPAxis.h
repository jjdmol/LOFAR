// Copyright notice should go here

#if !defined(UVPAXIS_H)
#define UVPAXIS_H

// $Id$

// Includes
#include <string>

#if(DEBUG_MODE)
#include <cassert>
#endif


//! Manages the properties of a coordinate axis of a dataset.
/*!  The UVPAxis class is used to scale and shift coordinates. One of
  its applications is the mapping of a value in the real world to a
  value in the pixel or intensity domain. For example for making graphs
  of data. The transfer function is: world = itsOffset +
  itsScale*axis. The inverse function is of course; axis = (world -
  itsOffset)/itsScale. Therefore itsScale may never have a zero value.
 */
class UVPAxis
{
 public:

  //! Constructor.
  /*! \param scale sets itsScale. It may not be 0.0.
      \param offset sets itsOffset. It may have any value.
      \param type indicates what is plotted along the axis and 
      \param unit the units used. */

   UVPAxis(double scale  = 1.0,
           double offset = 0.0,
           const std::string& type="",
           const std::string& unit="");


   //! Sets the transferfunction for this axis: world = offset + scale*axis.
   /* \param scale may not be 0.0.
      \param offset may have any value.
    */
  void        setTransferFunction(double scale,
                                  double offset);


  //! Calculates scale and offset of the transfer function from the
  //! provided world- and axis-ranges.

  /*! calcTransferFunction calculates itsScale and itsOffset fromn the
    world and axis ranges that are provided by the caller.
    \param axisMin and 
    \param axisMax may be pixel coordinates on a screen. axisMin may
    not be equal to axisMax.
    \param worldMin may not be equal to
    \param worldMax.
  */
  void        calcTransferFunction(double worldMin, 
                                   double worldMax,
                                   double axisMin,
                                   double axisMax);

  //! axis = (world - itsOffset)/itsScale
  /*! \param world is the real-world value of the coordinate.
      \returns the corresponding value on the axis.
   */
  inline double      worldToAxis(double world) const;

  //! world = itsOffset + itsScale*axis
  /*! \param axis is the value of the coordinate on the axis. For
       example a pixel coordinate.
       \returns the corresponding real-world value.
   */
  inline double      axisToWorld(double axis) const;

  //! \returns itsScale.
  double      getScale() const;

  //! \returns itsOffset.
  double      getOffset() const;

  //! \returns itsType.
  std::string getType() const;

  //! \returns itsUnit.
  std::string getUnit() const;



 private:
  
  double      itsScale;
  double      itsInverseScale;
  double      itsOffset;

  std::string itsType;
  std::string itsUnit;
};





//====================>>>  UVPAxis::worldToAxis  <<<====================

inline double UVPAxis::worldToAxis(double world) const
{
  return (world - itsOffset)*itsInverseScale;
}






//====================>>>  UVPAxis::axisToWorld  <<<====================

inline double UVPAxis::axisToWorld(double axis) const
{
  return itsOffset + axis*itsScale;
}

#endif // UVPAXIS_H
