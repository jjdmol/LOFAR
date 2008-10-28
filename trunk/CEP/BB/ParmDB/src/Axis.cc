//# Axis.cc: Classes representing a regular or irregular axis.
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

#include <lofar_config.h>
#include <ParmDB/Axis.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobSTL.h>
#include <Common/LofarLogger.h> 
#include <Common/StreamUtil.h> 
#include <casa/BasicMath/Math.h>

namespace LOFAR {
namespace BBS {

  using LOFAR::operator<<;

  // Initialize static.
  uint Axis::theirId = 0;


  // Register with the BlobStreamableFactory. Use an anonymous namespace. This
  // ensures that the 'dummy' variables get their own private storage area and
  // are only visible in this compilation unit.
  namespace
  {
    bool dummy1 = BlobStreamableFactory::instance().registerClass<RegularAxis>("RegularAxis");
    bool dummy2 = BlobStreamableFactory::instance().registerClass<OrderedAxis>("OrderedAxis");
    bool dummy3 = BlobStreamableFactory::instance().registerClass<UnorderedAxis>("UnorderedAxis");
  }


  Axis::Axis()
  {
    itsId = theirId++;
  }

  bool Axis::checkIntervals (const Axis& that) const
  {
    pair<double,double> range1 = range();
    pair<double,double> range2 = that.range();
    size_t index;
    Axis::ShPtr ax1 (subset(range2.first, range2.second, index));
    Axis::ShPtr ax2 (that.subset(range1.first, range1.second, index));
    uint nr = ax1->size();
    if (ax2->size() != nr) {
      return false;
    }
    for (uint i=0; i<nr; ++i) {
      double low1 = ax1->lower(i);
      double low2 = ax2->lower(i);
      if (low1 != low2  &&  !casa::near(low1, low2)) return false;
      double upp1 = ax1->upper(i);
      double upp2 = ax2->upper(i);
      if (upp1 != upp2  &&  !casa::near(upp1, upp2)) return false;
    }
    return true;
  }

  Axis::ShPtr Axis::combine (const Axis& that,
                             int& s1, int& e1, int& s2, int& e2) const
  {
    pair<double,double> range1 = range();
    pair<double,double> range2 = that.range();
    if (range1.second <= range2.first  ||
        casa::near(range1.second, range2.first)) {
      // this is fully left of that.
      Axis::ShPtr newAxis (add (that));
      s1 = 0;
      e1 = size();
      e2 = newAxis->size();
      s2 = e2 - that.size();
      return newAxis;
    }
    if (range2.second <= range1.first  ||
        casa::near(range2.second, range1.first)) {
      // that is fully left of this.
      Axis::ShPtr newAxis (that.add (*this));
      e1 = newAxis->size();
      s1 = e1 - size();
      s2 = 0;
      e2 = that.size();
      return newAxis;
    }
    // Full or partial overlap.
    int nr1 = size();
    int nr2 = that.size();
    double sc1 = center(0);
    double ec1 = center(nr1-1);
    double sc2 = that.center(0);
    double ec2 = that.center(nr2-1);
    // Find out where the range starts are.
    if (range1.first < range2.first) {
      s1 = 0;
      s2 = locate(sc2);
    } else {
      s1 = that.locate(sc1);
      s2 = 0;
    }
    // Find out how many intervals are part of the overlap.
    int nr;
    if (range1.second > range2.second) {
      nr = locate(ec2) - s2;
    } else {
      nr = that.locate(ec1) - s1;
    }
    ++nr;
    // Check if the overlapping parts match.
    for (int i=0; i<nr; ++i) {
      double low1 = lower(s2+i);
      double upp1 = upper(s2+i);
      double low2 = that.lower(s1+i);
      double upp2 = that.upper(s1+i);
      ASSERTSTR ((low1==low2 || casa::near(low1,low2))  &&
                 (upp1==upp2 || casa::near(upp1,upp2)),
                 "Axis::combine: interval [" << low1 << ',' << upp1
                 << "] mismatches [" << low2 << ',' << upp2 << ']');
    }
    // If this fully covers that, return this axis.
    if (s1 == 0  &&  nr == nr2) {
      e1 = nr1;
      e2 = s2+nr;
      return Axis::ShPtr (this->clone());
    }
    // If that fully covers this, return that axis and set which parts
    // of that are not contained in this.
    if (s2 == 0  &&  nr == nr1) {
      e1 = s1+nr;
      e2 = nr2;
      return Axis::ShPtr (that.clone());
    }
    // Partial overlap, so make a new axis.
    vector<double> low, upp;
    if (s1 == 0) {
      // this starts before that.
      e1 = nr1;
      e2 = s2+nr2;
      low.reserve (e2);
      upp.reserve (e2);
      for (int i=0; i<s2; ++i) {
        low.push_back (lower(i));
        upp.push_back (upper(i));
      }
      for (int i=0; i<nr2; ++i) {
        low.push_back (that.lower(i));
        upp.push_back (that.upper(i));
      }
    } else {
      // that starts before this.
      e1 = s1+nr1;
      e2 = nr2;
      low.reserve (e1);
      upp.reserve (e1);
      for (int i=0; i<s1; ++i) {
        low.push_back (that.lower(i));
        upp.push_back (that.upper(i));
      }
      for (int i=0; i<nr1; ++i) {
        low.push_back (lower(i));
        upp.push_back (upper(i));
      }
    }
    return makeAxis (low, upp);
  }

  Axis::ShPtr Axis::add (const Axis& that) const
  {
    // That axis will be appended to this one.
    pair<double,double> range1 = range();
    pair<double,double> range2 = that.range();
    int nr1 = size();
    int nr2 = that.size();
    vector<double> low, upp;
    low.reserve (nr1+nr2+1);
    upp.reserve (nr1+nr2+1);
    // Copy the first axis bounds.
    for (int i=0; i<nr1; ++i) {
      low.push_back (lower(i));
      upp.push_back (upper(i));
    }
    // See if there is a hole between the axes.
    // If so, create an extra interval for the hole.
    if (! casa::near (range1.second, range2.first)) {
      // Check this is before that.
      ASSERT (range1.second < range2.first);
      low.push_back (upp[nr1-1]);
      upp.push_back (that.lower(0));
    }
    // Copy the second axis bounds.
    for (int i=0; i<nr2; ++i) {
      low.push_back (that.lower(i));
      upp.push_back (that.upper(i));
    }
    return makeAxis (low, upp);
  }

  Axis::ShPtr Axis::makeAxis (const vector<double>& low,
                              const vector<double>& upp)
  {
    // Check if the width is constant, thus if the result is a regular axis.
    double width = upp[0] - low[0];
    for (uint i=1; i<low.size(); ++i) {
      if (!casa::near (width, upp[i]-low[i])) {
        return Axis::ShPtr (new OrderedAxis (low, upp, true));
      }
    }
    return Axis::ShPtr (new RegularAxis (low[0], width, low.size()));
  }



  RegularAxis::RegularAxis()
    : itsBegin(-1e30),
      itsWidth(2e30),
      itsCount(1)
  {}     
    
  RegularAxis::RegularAxis (double begin, double width, uint count,
                            bool asStartEnd)
    : itsBegin(begin),
      itsWidth(width),
      itsCount(count)
  {
    if (asStartEnd) {
      itsWidth = (itsWidth - itsBegin) / itsCount;
    }
    ASSERT(itsWidth > 0 && itsCount > 0);
  }

  RegularAxis::~RegularAxis()
  {}

  RegularAxis* RegularAxis::clone() const
  {
    return new RegularAxis(*this);
  }

  bool RegularAxis::operator== (const Axis& that) const
  {
    const RegularAxis* axis = dynamic_cast<const RegularAxis*>(&that);
    return (axis!=0 && itsBegin==axis->itsBegin && itsWidth==axis->itsWidth
            && itsCount==axis->itsCount);
  }

  bool RegularAxis::isRegular() const
  {
    return true;
  }

  bool RegularAxis::isOrdered() const
  {
    return true;
  }

  double RegularAxis::center (size_t n) const
  {
    return itsBegin + (n+0.5) * itsWidth;
  }
 
  double RegularAxis::lower (size_t n) const
  {
    return itsBegin + n * itsWidth;
  }

  double RegularAxis::upper (size_t n) const
  {
    return itsBegin + (n+1) * itsWidth;
  }

  double RegularAxis::width (size_t) const
  {
    return itsWidth;
  }

  size_t RegularAxis::size() const
  {
    return itsCount;
  }

  pair<double, double> RegularAxis::range() const
  { 
    return make_pair(lower(0), upper(size() - 1));
  }

  size_t RegularAxis::locate (double x, bool biasRight, size_t) const
  {
    // Find the cell that contains x.
    // A value not in the axis domain gets the first or last cell.
    double inxd = (x - itsBegin) / itsWidth;
    int inx = int(inxd);
    int last = int(itsCount) - 1;
    if (inx < 0) return 0;
    if (inx > last) return last;
    // If near the border of the cell, use left or right cell depending
    // on the biasRight argument.
    if (biasRight) {
      if (inx < last) {
        if (casa::near (double(inx+1), inxd)) ++inx;
      }
    } else {
      if (inx > 0) {
        if (casa::near(double(inx), inxd)) --inx;
      }
    }
    return inx;
  }

  Axis::ShPtr RegularAxis::subset (double start, double end,
                                   size_t& index) const
  {
    pair<double,double> rng = range();
    int sinx = 0;
    int einx = size() - 1;
    if (start > rng.first) {
      sinx = locate (start, true);
    }
    if (end < rng.second) {
      einx = locate (end, false);
    }
    index = sinx;
    return Axis::ShPtr (new RegularAxis (lower(sinx), itsWidth, 1+einx-sinx));
  }

  Axis::ShPtr RegularAxis::subset (size_t start, size_t end) const
  {
    if (start > end  ||  start >= size()  ||  end >= size()) {
      return Axis::ShPtr (new RegularAxis());
    }
    return Axis::ShPtr (new RegularAxis (itsBegin + start*itsWidth,
                                         itsWidth, end - start + 1));
  }

  Axis::ShPtr RegularAxis::compress (size_t factor) const
  {
    // Is the resulting axis still regular?
    if (itsCount % factor == 0) {
      return Axis::ShPtr(new RegularAxis(itsBegin, itsWidth * factor,
                                         itsCount / factor));
    }
    vector<double> centers(itsCount / factor + 1);
    for (size_t i = 0; i < itsCount / factor; ++i) {
      centers[i] = itsBegin + (i + 0.5) * factor * itsWidth;
    }
    centers.back() = lower(itsCount - (itsCount % factor)) +
      0.5 * (itsCount % factor) * itsWidth;
    vector<double> widths(itsCount / factor + 1, factor * itsWidth);
    widths.back() = (itsCount % factor) * itsWidth;
    return Axis::ShPtr(new OrderedAxis(centers, widths));
  }


  void RegularAxis::write (BlobOStream& bos) const
  {
    bos << itsBegin << itsWidth << itsCount;
  }

  void RegularAxis::read (BlobIStream& bis)
  {
    bis >> itsBegin >> itsWidth >> itsCount;
  }

  const string& RegularAxis::classType() const
  {
    static string type("RegularAxis");
    return type;
  }



  IrregularAxis::IrregularAxis()
    : itsCenters (1, 0.),
      itsHWidths (1, 1e30)
  {}

  IrregularAxis::IrregularAxis (const vector<double>& starts,
                                const vector<double>& ends,
                                bool asStartEnd,
                                bool checkOrder)
  {
    ASSERT(starts.size() == ends.size());
    ASSERT(starts.size() > 0);
    uint nr = starts.size();
    if (!asStartEnd) {
      itsCenters = starts;
      itsHWidths = ends;
      for (uint i=0; i<nr; ++i) {
        itsHWidths[i] *= 0.5;
      }
    } else {
      itsCenters.reserve (nr);
      itsHWidths.reserve (nr);
      for (uint i=0; i<nr; ++i) {
        itsCenters.push_back ((ends[i] + starts[i]) * 0.5);
        itsHWidths.push_back ((ends[i] - starts[i]) * 0.5);
      }
    }
    for (uint i=0; i<nr; ++i) {
      ASSERT (itsHWidths[i] > 0);
      if (checkOrder  &&  i > 0) {
        double end = itsCenters[i-1] + itsHWidths[i-1];
        double st  = itsCenters[i]   - itsHWidths[i];
        ASSERT(st>=end  ||  casa::near(st,end));
      }
    }
  }

  IrregularAxis::~IrregularAxis()
  {}
    
  bool IrregularAxis::operator== (const Axis& that) const
  {
    if (classType() != that.classType()) return false;
    const IrregularAxis& axis = dynamic_cast<const IrregularAxis&>(that);
    return itsCenters==axis.itsCenters && itsHWidths==axis.itsHWidths;
  }

  bool IrregularAxis::isRegular() const
  {
    return false;
  }

  double IrregularAxis::center (size_t n) const
  {
    return itsCenters[n];
  }
 
  double IrregularAxis::lower (size_t n) const
  {
    return itsCenters[n] - itsHWidths[n];
  }

  double IrregularAxis::upper (size_t n) const
  {
    return itsCenters[n] + itsHWidths[n];
  }

  double IrregularAxis::width (size_t n) const
  {
    return 2*itsHWidths[n];
  }

  size_t IrregularAxis::size() const
  {
    return itsCenters.size();
  }

  pair<double, double> IrregularAxis::range() const
  { 
    return make_pair(lower(0), upper(itsCenters.size() - 1));
  }

  void IrregularAxis::write (BlobOStream& bos) const
  {
    bos << itsCenters << itsHWidths;
  }

  void IrregularAxis::read (BlobIStream& bis)
  {
    bis >> itsCenters >> itsHWidths;
  }



  OrderedAxis::OrderedAxis()
  {}

  OrderedAxis::OrderedAxis (const vector<double>& starts,
                            const vector<double>& ends,
                            bool asStartEnd)
    : IrregularAxis (starts, ends, asStartEnd, true)
  {}

  OrderedAxis::~OrderedAxis()
  {}
    
  OrderedAxis* OrderedAxis::clone() const
  {
    return new OrderedAxis(*this);
  }

  bool OrderedAxis::isOrdered() const
  {
    return true;
  }

  size_t OrderedAxis::locate (double x, bool biasRight, size_t start) const
  {
    size_t nr = itsCenters.size();
    // Start searching at the given start position.
    // Start at the beginning if needed.
    if (start >= nr  ||  x < itsCenters[start]-itsHWidths[start]) {
      start = 0;
    }
    while (true) {
      double s = itsCenters[start]-itsHWidths[start];
      double e = itsCenters[start]+itsHWidths[start];
      if (x < s) {
        if (biasRight  &&  casa::near(x,s)) {
          // On the left edge.
          break;
        }
        // Value before first interval uses first interval.
        ASSERTSTR (start==0, "No interval found for value " << x);
        break;
      } else if (x < e) {
        // Inside the interval.
        break;
      } else {
        if (!biasRight) {
          if (casa::near(x,e)) {
            // On the right edge.
            break;
          }
        } else if (start < nr-1
                   && !casa::near(x, itsCenters[start+1]-itsHWidths[start+1])) {
          if (casa::near(x,e)) {
            // On the right edge and not on left edge of next one.
            break;
          }
        }
      }
      if (start == nr-1) {
        // Value after last interval uses last interval.
        break;
      }
      ++start;
    }
    return start;
  }

  Axis::ShPtr OrderedAxis::subset (double start, double end,
                                   size_t& index) const
  {
    ASSERT (start < end);
    size_t sinx = locate (start, true);
    size_t einx = locate (end, false, sinx);
    vector<double> centers(itsCenters.begin()+sinx, itsCenters.begin()+einx+1);
    vector<double> widths (itsHWidths.begin()+sinx, itsHWidths.begin()+einx+1);
    for (uint i=0; i<widths.size(); ++i) {
      widths[i] *= 2.;
    }
    index = sinx;
    return Axis::ShPtr (new OrderedAxis (centers, widths));
  }

  Axis::ShPtr OrderedAxis::subset(size_t start, size_t end) const
  {
    if (start > end  ||  start >= size()  ||  end >= size()) {
      return Axis::ShPtr(new OrderedAxis());
    }
    size_t size = end - start + 1;
    vector<double> centers(size);
    vector<double> widths(size);    
    for (size_t i=0; i<size; ++i) {
      centers[i] = itsCenters[start + i];
      widths[i]  = itsHWidths[start + i] * 2.0;
    }
    return Axis::ShPtr(new OrderedAxis(centers, widths));
  }

  Axis::ShPtr OrderedAxis::compress (size_t) const
  {
    ASSERTSTR(false, "UnorderedAxis::compress not implemented");
  }
    
  const string& OrderedAxis::classType() const
  {
    static string type("OrderedAxis");
    return type;
  }



  UnorderedAxis::UnorderedAxis()
  {}

  UnorderedAxis::UnorderedAxis (const vector<double>& starts,
                                const vector<double>& ends,
                                bool asStartEnd)
    : IrregularAxis (starts, ends, asStartEnd, false)
  {}

  UnorderedAxis::~UnorderedAxis()
  {}

  UnorderedAxis* UnorderedAxis::clone() const
  {
    return new UnorderedAxis(*this);
  }

  bool UnorderedAxis::isOrdered() const
  {
    return false;
  }

  size_t UnorderedAxis::locate (double x, bool biasRight, size_t start) const
  {
    size_t nr = itsCenters.size();
    // Start searching at the given start position.
    // Start at the beginning if needed.
    if (start >= nr) {
      start = 0;
    }
    // A possible match.
    int possible = -1;
    size_t startOrig = start;
    while (true) {
      double s = itsCenters[start]-itsHWidths[start];
      double e = itsCenters[start]+itsHWidths[start];
      if (x < s) {
        if (casa::near(x,s)) {
          // On the left edge.
          if (biasRight) break;
          possible = start;
        }
      } else if (x < e) {
        // Inside the interval.
        break;
      } else {
        if (casa::near(x,e)) {
          // On the right edge.
          if (!biasRight) break;
          possible = start;
        }
      }
      ++start;
      if (start == nr) {
        start = 0;
      }
      // If not found, return possible match, otherwise first or last interval.
      if (start == startOrig) {
        if (possible >= 0) {
          start= possible;
        } else {
          if (x < itsCenters[0]) {
            start = 0;
          } else {
            start = nr-1;
          }
        }
        break;
      }
    }
    return start;
  }

  Axis::ShPtr UnorderedAxis::subset (double start, double end,
                                     size_t& index) const
  {
    vector<double> cen, wid;
    index = 0;
    bool first = true;
    // Use all intervals containing a part of start-end.
    for (size_t i=0; i<itsCenters.size(); ++i) {
      if (start <= itsCenters[i]+itsHWidths[i]  &&
          end   >= itsCenters[i]-itsHWidths[i]) {
        cen.push_back (itsCenters[i]);
        wid.push_back (2*itsHWidths[i]);
        if (first) {
          index = i;
          first = false;
        }
      }
    }
    return Axis::ShPtr (new UnorderedAxis (cen, wid));
  }

  Axis::ShPtr UnorderedAxis::subset(size_t start, size_t end) const
  {
    if (start > end  ||  start >= size()  ||  end >= size()) {
      return Axis::ShPtr(new UnorderedAxis());
    }
    size_t size = end - start + 1;
    vector<double> centers(size);
    vector<double> widths(size);    
    for (size_t i=0; i<size; ++i) {
      centers[i] = itsCenters[start + i];
      widths[i]  = itsHWidths[start + i] * 2.0;
    }
    return Axis::ShPtr(new UnorderedAxis(centers, widths));
  }

  Axis::ShPtr UnorderedAxis::compress (size_t) const
  {
    ASSERTSTR(false, "UnorderedAxis::compress not implemented");
  }
    
  const string& UnorderedAxis::classType() const
  {
    static string type("UnorderedAxis");
    return type;
  }


} // namespace BBS
} // namespace LOFAR
