// Copyright notice

#if !defined(UVPDATAATOM_H)
#define UVPDATAATOM_H

// $Id$


#include <vector>
#include <complex>

#include <UVPDataAtomHeader.h>

//! Manages a vector of visibilities recorded for one correlation, on
//! one baseline at a given time.
/*!  The uvw coordinates are in meters. The visibility data in
  Jansky. This is the basic data unit of a synthesis observation as we
  currently consider it.
 */
class UVPDataAtom
{
 public:

  typedef float_complex ComplexType;
  
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
  
  //! Assigns one datapoint to a channel.
  /*!
    \param channel is a zero based channel index.
    \param data  is the visibility that is to be set.
   */
  void            setData(unsigned int          channel,
                          const ComplexType& data);
  
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


  //! \returns the number of channels.
  unsigned int getNumberOfChannels() const;

  //! \returns a pointer to a specific visibilty.
  /*!
      \param channel is a zero based channel index. 
   */
  const ComplexType *getData(unsigned int channel) const;


  //! \returns const ref to itsHeader.
  const UVPDataAtomHeader &getHeader() const;

 protected:
 private:

  UVPDataAtomHeader        itsHeader;
  std::vector<ComplexType> itsData;
};

#endif // UVPDATAATOM_H
