//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(UVPDATAATOM_H)
#define UVPDATAATOM_H

// $Id$


#include <vector>
#include <complex>

#include <uvplot/UVPDataAtomHeader.h>

//! Manages a vector of visibilities recorded for one correlation, on
//! one baseline at a given time.
/*!  The uvw coordinates are in meters. The visibility data in
  Jansky. This is the basic data unit of a synthesis observation as we
  currently consider it.
 */
class UVPDataAtom
{
 public:

  typedef std::complex<float>               ComplexType;
  
  typedef std::vector<bool>::const_iterator FlagIterator;

  //! Default constructor.
  /*!  The header is initialized with its default constructor, The data
       array initially has zero length.
   */
  UVPDataAtom();

  //! Constructor.
  /*!
    \param numberOfChannels must be >= 0.
    \param header contains additional data like antenna numbers, int. time etc.
  */
  UVPDataAtom(unsigned int               numberOfChannels,
              const UVPDataAtomHeader&   header);

  //! Copy constructor.
  /*! Slightly dirty, should be faster than the one supplied by the compiler.
   */
  UVPDataAtom(const UVPDataAtom &other);


  //! Sets a new number of channels.
  /*! setChannels destroys the previous contents of itsData and
    allocates new storage for numberOfChannels channels.
   */
  void            setChannels(unsigned int numberOfChannels);
  


  //! Sets a new header
  /*!
   */
  void            setHeader(const UVPDataAtomHeader& header);
  
  //! Assigns one datapoint to a channel.
  /*!
    \param channel is a zero based channel index.
    \param data  is the visibility that is to be set.
   */
  void            setData(unsigned int          channel,
                          const ComplexType&    data);
  
  //! Assigns a complete vector of visibilities to itsData
  /*! setData overwrites itsData with the contents of data.

       \param data is the list of visibilities that are to be assigned
       to itsData. It must have the exact same size as the current
       number of channels.
   */
  void            setData(const std::vector<ComplexType>& data);


  //! Assigns a complete array of visibilities to itsData
  /*! setData overwrites the contents of itsData with the contents of
    data. This is the fastest of all setData routines.

       \param data is the list of visibilities that are to be assigned
       to itsData. It must have the exact same size as the current
       number of channels.
   */
  void            setData(const ComplexType* data);

  //! Sets the flags.
  /*! \param flags must have length equal to the number of channels in
  the data.
  */
  void            setFlags(const std::vector<bool>& flags);

  //! Sets one flag to "flag"
  void            setFlag(unsigned int channel,
                          bool         flag);

  //! \returns the number of channels.
  unsigned int    getNumberOfChannels() const;

  //! \returns a pointer to a specific visibility.
  /*!
      \param channel is a zero based channel index. 
   */
  const ComplexType* getData(unsigned int channel) const;


  //! \returns the value of a specific flag.
  /*! 
      \param channel is a zero based channel index. 
   */
  const bool         getFlag(unsigned int channel) const;

  //! \returns const_iterator to first flag.
  FlagIterator       getFlagBegin() const;

  //! \returns const_iterator one past last flag.
  FlagIterator       getFlagEnd() const;

  //! \returns const ref to itsHeader.
  const UVPDataAtomHeader& getHeader() const;


  //! Stores internal state in binary format.
  /*! Format:
    - itsHeader
    - unsigned int (4 bytes) number of channels
    - N*8 bytes  std::complex<float>  data
    - N*1 bytes  unsigned char        flags (1(T) or 0(F))
   */
  void               store(std::ostream& out) const;

  //! Loads internal state in binary format.
  void               load(std::istream& in);


 protected:
 private:

  UVPDataAtomHeader        itsHeader;
  std::vector<ComplexType> itsData;

  //! True if corresponding datapoint is flagged.
  std::vector<bool>        itsFlags;
};

#endif // UVPDATAATOM_H
