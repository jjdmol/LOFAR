// Copyright notice should go here

// $ID$

#include <uvplot/UVPPixel.h>


#if(DEBUG_MODE)
#include <cassert>
#endif


//====================>>>  UVPPixel::UVPPixel  <<<====================

UVPPixel::UVPPixel()
  :itsAverageValue(0),
   itsValue(0),
   itsWeight(0),
   itsRowIndex(0)
{
}





#if(ADD_POINT_TRUECALKJHD)
//====================>>>  UVPPixel::addPoint  <<<====================

bool UVPPixel::addPoint(double value,
                        int    rowIndex,
                        double weight)
{
  #if(DEBUG_MODE)
  assert(weight > 0);
  #endif

  if(weight <= 0.0) {
    return false;
  }

  double cumulativeWeight = 0.0;

  itsValue.push_back(value);
  itsWeight.push_back(weight);
  itsRowIndex.push_back(rowIndex);

  itsAverageValue = 0.0;

  unsigned int N = itsValue.size();
  for(unsigned int i = 0; i < N; i++) {
    cumulativeWeight += itsWeight[i];
    itsAverageValue  += itsWeight[i]*itsValue[i];
  }

  // The requirement that weight > 0.0 makes sure that
  // cumulativeWeight is always > 0.0
  itsAverageValue /= cumulativeWeight;
  
  return true;
}
#endif








//====================>>>  UVPPixel::addPointUniform  <<<====================

bool UVPPixel::addPointUniform(double value,
                               int    rowIndex)
{
  int    NumberOfPoints = itsRowIndex.size();

  itsRowIndex.push_back(rowIndex);

  itsAverageValue *= NumberOfPoints;
  itsAverageValue += value;
  itsAverageValue /= (NumberOfPoints+1);


  return true;
}









//====================>>>  UVPPixel::getRowIndex  <<<====================

std::vector<int> UVPPixel::getRowIndex() const
{
  return itsRowIndex;
}
