// Copyright notice

// $Id$

#if !defined(UVPDATAATOM_H)
#define UVPDATAATOM_H


#include <vector>
#include <complex>



//! Manages a vector of visibilities recorded for one correlation, on
//! one baseline at a given time.
/*!  The uvw coordinates are in meters. The visibility data in
  Jansky. This is the basic data unit of a synthesis observation as we
  currently consider it.
 */
class UVPDataAtom
{
 public:
  
  //! Default constructor.
  /*!  UVW coordinates are initialized to 0, The data array initially
    has zero length, time is 0.
   */
  UVPDataAtom();

  //! Constructor.
  /*!
    \param numberOfChannels must be >= 0.
    \param time is the time in seconds. The user is responsible for
    defining the exact zeropoint.
    \param uvw is a vector of length 3 specifying the u (0), v (1) and
    w (2) coordinates of the visibility spectrum in meters. 
   */
  UVPDataAtom(unsigned int             numberOfChannels,
              double                     time,
              const std::vector<double> &uvw);

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
  void            setData(unsigned int     channel,
                          const double_complex& data);
  
  //! Assigns a complete vector of visibilities to itsData
  /*! setData overwrites itsData with the contents of data.

       \param data is the list of visibilities that are to be assigned
       to itsData. It must have the exact same size as the current
       number of channels.
   */
  void            setData(const std::vector<double_complex>& data);

  //! Sets new UVW coordinates.
  /*!
    \param uvw must be an array of length 3. u is at index 0, v at 1
    and w at index 2.
   */
  void            setUVW(const std::vector<double> &uvw);


  //! \returns the number of channels.
  unsigned int getNumberOfChannels() const;

  //! \returns a pointer to a specific visibilty.
  /*!
      \param channel is a zero based channel index. 
   */
  const double_complex *getData(unsigned int channel) const;

 protected:
 private:
  
  std::vector<double_complex> itsData;
  double                   itsTime;
  std::vector<double>      itsUVW;
};

#endif // UVPDATAATOM_H
