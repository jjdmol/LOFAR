//# Axis.h: Classes representing a regular or irregular axis.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

// @file
// @brief Classes representing a regular or irregular axis.
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_AXIS_H
#define LOFAR_PARMDB_AXIS_H

#include <Blob/BlobStreamable.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
namespace BBS {

  using std::pair;
  using std::make_pair;

  // @ingroup ParmDB
  // @{

  // @brief Abstract base class for a cell centered axis.
  class Axis: public BlobStreamable
  {
  public:
    typedef shared_ptr<Axis> ShPtr;

    // The constructor sets the unique id.
    Axis();

    virtual ~Axis()
    {}

    // Clone the object.
    virtual Axis* clone() const = 0;

    // Check if two axes are equal. They are if they the same type and values.
    // <group>
    virtual bool operator== (const Axis& that) const = 0;
    bool operator!= (const Axis& that) const
      { return ! operator== (that); }
    // </group>

    // Get the unique axis id.
    uint getId() const
      { return itsId; }

    // Get the center, etc. of the i-th cell.
    // <group>
    virtual double center(size_t n) const = 0;
    virtual double lower(size_t n) const = 0;
    virtual double upper(size_t n) const = 0;
    virtual double width(size_t n) const = 0;
    // </group>

    // Is the axis regular?
    virtual bool isRegular() const = 0;

    // Is the axis ordered?
    virtual bool isOrdered() const = 0;

    // Get nr of cells.
    virtual size_t size() const = 0;

    // Get the total range of the axis.
    virtual pair<double, double> range() const = 0;

    // Get the start and end value.
    // <group>
    double start() const
      { return range().first; }
    double end() const
      { return range().second; }
    // </group>

    // Get the cellnr of the cell containing value x.
    // If x is right on the edge, biasRight tells if the left or right cell
    // is taken.
    // As a search hint one can tell where to start the search (e.g. the
    // result of the previous locate).
    virtual size_t locate(double x, bool biasRight = true,
                          size_t start=0) const = 0;

    // Check if the corresponding intervals in this and that axis are the same.
    bool checkIntervals (const Axis& that) const;

    // Make a subset of the axis for the given start/end value.
    // It fills the index of the starting point of the subset on the axis.
    virtual Axis::ShPtr subset (double start, double end,
                                size_t& index) const = 0;

    // Make a subset of the axis for the given start/end index.
    virtual Axis::ShPtr subset (size_t start, size_t end) const = 0;

    // Compress the axis.
    virtual Axis::ShPtr compress(size_t factor) const = 0;

    // Return the union of this and that axis.
    // If checks if matching intervals are the same.
    // It fills s1,e1 with the first and last index of this axis in the new
    // one. Similarly s2,e2 are filled for that axis.
    // Note the e1 and e2 are one past the end.
    // The returned object is a RegularAxis if all intervals in the result
    // are consecutive and have the same width, otherwise the result
    // is an OrderedAxis.
    Axis::ShPtr combine (const Axis& that,
                         int& s1, int& e1, int& s2, int& e2) const;

    // Return the type of \c *this as a string.
    virtual const string& classType() const = 0;

  private:
    // Add this and that axis, where this axis must be before that axis.
    Axis::ShPtr add (const Axis& that) const;

    // Make an Axis object from the intervals defined by the low/upp values.
    // If all intervals have the same width, a RegularAxis object is made.
    // Otherwise an OrderedAxis object.
    // The intervals must be consecutive.
    static Axis::ShPtr makeAxis (const vector<double>& low,
                                       const vector<double>& high);

    //# Unique seqnr of an Axis object. Used in class AxisMapping.
    uint        itsId;
    static uint theirId;
  };


  // @brief Regularly strided cell centered axis.
  class RegularAxis: public Axis
  {
  public:
    // Default constructor creates one cell from -1e30 till 1e30.
    RegularAxis();

    // Construct giving the beginning of the axis and the width of each cell.
    RegularAxis(double begin, double cellWidth, uint count,
                bool asStartEnd=false);

    virtual ~RegularAxis();

    // Clone the object.
    virtual RegularAxis* clone() const;

    // Check if two axes are equal. They are if they the same type and values.
    virtual bool operator== (const Axis& that) const;

    // The axis is regular.
    virtual bool isRegular() const;

    // The axis is ordered.
    virtual bool isOrdered() const;

    virtual double center(size_t n) const;
    virtual double lower(size_t n) const;
    virtual double upper(size_t n) const;
    virtual double width(size_t) const;
    virtual size_t size() const;
    virtual pair<double, double> range() const;
    virtual size_t locate(double x, bool biasRight = true,
                          size_t start=0) const;
    virtual Axis::ShPtr subset (double start, double end, size_t& index) const;
    virtual Axis::ShPtr subset (size_t start, size_t end) const;
    virtual Axis::ShPtr compress(size_t factor) const;
    
  private:
    // Write the contents of \c *this into the blob output stream \a bos.
    virtual void write(BlobOStream& bos) const;

    // Read the contents from the blob input stream \a bis into \c *this.
    virtual void read(BlobIStream& bis);

    // Return the type of \c *this as a string.
    virtual const string& classType() const;
    
    double  itsBegin, itsWidth;
    uint32  itsCount;
  };


  // @brief Base class for irregularly strided cell centered axis.
  class IrregularAxis: public Axis
  {
  public:
    // Default constructor creates one cell from -1e30 till 1e30.
    IrregularAxis();

    // Specify as v1/v2 as width/center or start/end.
    IrregularAxis(const vector<double>& v1, const vector<double>& v2,
                  bool asStartEnd, bool checkOrder);
    
    virtual ~IrregularAxis();

    // Check if two axes are equal. They are if they the same type and values.
    virtual bool operator== (const Axis& that) const;

    // The axis is not regular.
    virtual bool isRegular() const;

    virtual double center(size_t n) const;
    virtual double lower(size_t n) const;
    virtual double upper(size_t n) const;
    virtual double width(size_t n) const;
    virtual size_t size() const;
    virtual pair<double, double> range() const;

  protected:
    // Write the contents of \c *this into the blob output stream \a bos.
    virtual void write(BlobOStream& bos) const;

    // Read the contents from the blob input stream \a bis into \c *this.
    virtual void read(BlobIStream& bis);

    vector<double> itsCenters;
    vector<double> itsHWidths;
  };


  // @brief Ordered rrregularly strided cell centered axis.
  // The cells are ordered and disjoint, but gaps may be present.
  class OrderedAxis: public IrregularAxis
  {
  public:
    // Default constructor creates one cell from -1e30 till 1e30.
    OrderedAxis();

    // Specify the intervals defined by v1/v2 as width/center or start/end.
    // The vectors must have equal sizes. The intervals must be in ascending
    // order and they have to be disjoint. However, they do not need to be
    // consecutive. 
    OrderedAxis(const vector<double>& v1, const vector<double>& v2,
                bool asStartEnd=false);
    
    virtual ~OrderedAxis();

    // Clone the object.
    virtual OrderedAxis* clone() const;

    // The axis is ordered.
    virtual bool isOrdered() const;

    virtual size_t locate(double x, bool biasRight = true,
                          size_t start=0) const;
    virtual Axis::ShPtr subset (double start, double end, size_t& index) const;
    virtual Axis::ShPtr subset (size_t start, size_t end) const;
    virtual Axis::ShPtr compress(size_t factor) const;

  private:
    // Return the type of \c *this as a string.
    virtual const string& classType() const;
  };


  // @brief Unordered irregularly strided cell centered axis.
  // The cells are unordered and need not be disjoint; but gaps may be present.
  class UnorderedAxis: public IrregularAxis
  {
  public:
    // Default constructor creates one cell from -1e30 till 1e30.
    UnorderedAxis();

    // Specify the intervals defined by v1/v2 as width/center or start/end.
    // The vectors must have equal sizes. The intervals do not need to be
    // ordered nor do they have to be disjoint.
    UnorderedAxis(const vector<double>& v1, const vector<double>& v2,
                  bool asStartEnd=false);
    
    virtual ~UnorderedAxis();

    // Clone the object.
    virtual UnorderedAxis* clone() const;

    // The axis is not ordered.
    virtual bool isOrdered() const;

    virtual size_t locate(double x, bool biasRight = true,
                          size_t start=0) const;
    virtual Axis::ShPtr subset (double start, double end, size_t& index) const;
    virtual Axis::ShPtr subset (size_t start, size_t end) const;
    virtual Axis::ShPtr compress(size_t factor) const;

  private:
    // Return the type of \c *this as a string.
    virtual const string& classType() const;

    double itsStart;
    double itsEnd;
  };

  // @}

} //# namespace BBS
} //# namespace LOFAR

#endif
