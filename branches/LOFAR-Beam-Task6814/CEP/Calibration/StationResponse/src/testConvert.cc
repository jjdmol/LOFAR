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

#include <StationResponse/AntennaFieldHBA.h>

#include <vector>

using namespace std;
using namespace casa;

// Convert ra-dec (j2000) to itrf through casacore directly
vector<double> j2000_to_itrf(vector<double> &vec_pos,
                             vector<double> &vec_j2000) {
  MDirection dir_radec(
               MVDirection(
                 Quantity(vec_j2000[0], "rad"),
                 Quantity(vec_j2000[1],"rad")),
               MDirection::J2000);

  cout<<"dir_radec: "<<dir_radec<<endl;

  MEpoch time(Quantity(56788.02674768519,"d"),MEpoch::UTC);

  cout<<"time: "<<time<<endl;

  MPosition pos(
               MVPosition(
                 Quantity(0,"m"), 
                 Quantity(vec_pos[0],"deg"), 
                 Quantity(vec_pos[1], "deg")),
               MPosition::WGS84);

  cout<<"pos: "<<pos<<endl;

  MeasFrame frame1(time, pos);

  cout<<"frame1: "<<frame1<<endl;

  MDirection::Convert radec2itrfconverter(
                         dir_radec, 
                         MDirection::Ref(MDirection::ITRF, frame1));
  
  MDirection dir_itrf = radec2itrfconverter();

  cout<<"dir_itrf: "<<dir_itrf<<endl;

  vector<double> vec_itrf(3);
  vec_itrf[0]=dir_itrf.getValue()(0);
  vec_itrf[1]=dir_itrf.getValue()(1);
  vec_itrf[2]=dir_itrf.getValue()(2);
  return vec_itrf;
}

// Convert itrf to azel through casacore directly
vector<double> itrf_to_azel(vector<double> vec_pos, vector<double> vec_itrf) {
  MDirection dir_itrf(MVDirection(vec_itrf[0], vec_itrf[1], vec_itrf[2]), MDirection::ITRF);

  MPosition pos(
               MVPosition(
                 Quantity(0,"m"), 
                 Quantity(vec_pos[0],"deg"), 
                 Quantity(vec_pos[1], "deg")),
               MPosition::WGS84);

  cout<<"pos: "<<pos<<endl;

  MeasFrame frame2(pos);

  MDirection::Convert itrf2azel(
                         dir_itrf,
                         MDirection::Ref(MDirection::AZEL, frame2));

  MDirection dir_azel = itrf2azel();

  cout<<"dir_azel: "<<dir_azel.getAngle()<<endl;
  vector<double> vec_azel(2);
  vec_azel[0]=dir_azel.getAngle().getValue()[0];
  vec_azel[1]=dir_azel.getAngle().getValue()[1]; 
  return vec_azel;
}


int main(int, char **)
{
  vector<double> pos_vec(2);
  pos_vec[0]=52.911392;
  pos_vec[1]=6.867630;

  vector<double> j2000_dir(2);
  j2000_dir[0]=2.7574313416009946;
  j2000_dir[1]=0.12522052767963945;
 
  vector<double> itrf_dir;
 
  itrf_dir=j2000_to_itrf(pos_vec, j2000_dir);

  cout<<"ITRF vector: "<<itrf_dir[0]<<", "<<itrf_dir[1]<<", "<<itrf_dir[2]<<endl;

  vector<double> azel_dir;

  azel_dir=itrf_to_azel(pos_vec,itrf_dir);

  cout<<"AZ-EL vector: "<<azel_dir[0]<<", "<<azel_dir[1]<<endl;

  return 0;
}

