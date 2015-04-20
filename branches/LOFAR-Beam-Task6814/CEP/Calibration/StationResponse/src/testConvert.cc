#include <lofar_config.h>
#include <iostream>

#include <casacore/casa/aips.h>
#include <casacore/measures/Measures.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/measures/Measures/MEpoch.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/measures/Measures/MeasFrame.h>
#include <casacore/measures/Measures/MCDirection.h>
#include <casacore/measures/Measures/MeasConvert.h>


using namespace std;
using namespace casa;

int main(int, char **)
{
  MDirection dir_radec(
               MVDirection(
                 Quantity(2.7574313416009946, "rad"),
                 Quantity(0.12522052767963945,"rad")),
               MDirection::J2000);

  cout<<"dir_radec: "<<dir_radec<<endl;

  MEpoch time(Quantity(56788.02674768519,"d"),MEpoch::UTC);

  cout<<"time: "<<time<<endl;

  MPosition pos(
               MVPosition(
                 Quantity(0,"m"), 
                 Quantity(52.911392,"deg"), 
                 Quantity(6.867630, "deg")),
               MPosition::WGS84);

  cout<<"pos: "<<pos<<endl;

  MeasFrame frame1(time, pos);

  cout<<"frame1: "<<frame1<<endl;

  MDirection::Convert radec2itrfconverter(
                         dir_radec, 
                         MDirection::Ref(MDirection::ITRF, frame1));
  
  MDirection dir_itrf = radec2itrfconverter();

  cout<<"dir_itrf: "<<dir_itrf<<endl;

  MeasFrame frame2(pos);

  MDirection::Convert itrf2azel(
                         dir_itrf,
                         MDirection::Ref(MDirection::AZEL, frame2));

  MDirection dir_azel = itrf2azel();

  cout<<"dir_azel: "<<dir_azel.getAngle()<<endl;

  return 0;
}

