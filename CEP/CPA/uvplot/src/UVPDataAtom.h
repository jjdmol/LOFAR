// Copyright notice

// $ID

#if !defined(UVPDATAATOM_H)
#define UVPDATAATOM_H


#include <vector>
#include <complex>

//! Manages a vector of visibilities recorded for one correlation, on
//! one baseline at a given time.
/*!
  
 */
class UVPDataAtom
{
 public:
  
  UVPDataAtom(int    numberOfChannels = 0,
              double time            = 0);

  //! Sets a new number of channels.
  /*! setChannels destroys the previous contents of itsData.
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
};

#endif // UVPDATAATOM_H
