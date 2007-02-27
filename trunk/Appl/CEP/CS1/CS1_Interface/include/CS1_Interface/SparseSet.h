//#  SparseSet.h: portable <bitset> adaptation
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$


#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_BITSET_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_BITSET_H

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

#include <vector>


namespace LOFAR {

class SparseSet {
  public:
    struct range {
      range() {}
      range(unsigned begin, unsigned end) : begin(begin), end(end) {}
      unsigned begin, end;
    };

    SparseSet	&include(unsigned index);
    SparseSet	&include(unsigned first /* inclusive */, unsigned last /* exclusive */);

    SparseSet	&exclude(unsigned index);
    SparseSet	&exclude(unsigned first /* inclusive */, unsigned last /* exclusive */);

    void	reset();

    unsigned	count() const;
    bool	test(unsigned index) const;

    SparseSet	operator | (const SparseSet &) const;
    SparseSet	&operator |= (const SparseSet &);
    SparseSet	&operator += (size_t count);
    SparseSet	&operator -= (size_t count);
    SparseSet	subset(unsigned first, unsigned last) const;

    const std::vector<struct range> &getRanges() const;

    void write(BlobOStream &) const;
    void read(BlobIStream &);

    ssize_t marshall(void *ptr, size_t maxSize) const;
    void    unmarshall(const void *ptr);

  private:
    std::vector<struct range> ranges;
};

std::ostream &operator << (std::ostream &str, const SparseSet &set);


inline SparseSet &SparseSet::include(unsigned index)
{
  return include(index, index + 1);
}


inline SparseSet &SparseSet::exclude(unsigned index)
{
  return exclude(index, index + 1);
}


inline void SparseSet::reset()
{
  ranges.resize(0);
}


inline SparseSet &SparseSet::operator |= (const SparseSet &other)
{
  ranges = (*this | other).ranges;
  return *this;
}


inline SparseSet SparseSet::subset(unsigned first, unsigned last) const
{
  return SparseSet(*this).exclude(last, ~0U).exclude(0, first);
}


inline const std::vector<struct SparseSet::range> &SparseSet::getRanges() const
{
  return ranges;
}

} // namespace LOFAR

#endif
