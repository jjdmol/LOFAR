// Copyright notice

// $ID

#if !defined(UVPDATAATOM_H)
#define UVPDATAATOM_H


#include <vector>
#include <complex>

class UVPDataAtom
{
 public:
  
  UVPDataAtom(int    numberOfChannels = 0,
              double time            = 0);

  // Sets a new number of channels. Destroys previous contents of
  // itsData.
  void            setChannels(unsigned int numberOfChannels);
  
  void            setData(unsigned int     channel,
                          const double_complex& data);
  
  void            setData(const std::vector<double_complex>& data);

  unsigned int getNumberOfChannels() const;

  const double_complex *getData(unsigned int channel) const;

 protected:
 private:
  
  std::vector<double_complex> itsData;
  double                   itsTime;
};

#endif // UVPDATAATOM_H
