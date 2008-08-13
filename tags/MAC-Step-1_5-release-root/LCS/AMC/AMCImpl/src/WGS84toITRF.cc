#include <lofar_config.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MCPosition.h>
#include <casa/Exceptions/Error.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <Common/Exception.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

MPosition readPos()
{
  cout << endl << "Enter WGS84 coordinates: " << endl;
  double val;

  cout << "  longitude (deg): ";
  if (!(cin >> val)) THROW (Exception, "Error reading longitude!");
  Quantity longitude(val, "deg");

  cout << "  latitude (deg):  ";
  if (!(cin >> val)) THROW (Exception, "Error reading latitude!");
  Quantity latitude(val, "deg");

  cout << "  height (m):      ";
  if (!(cin >> val)) THROW (Exception, "Error reading height!");
  Quantity height(val, "m");

  return MPosition(height, longitude, latitude, MPosition::WGS84);
}


void showPos(const MPosition& pos, ostream& os = cout)
{
  os.precision(9);
  os << "Coordinates in "     << pos.getRefString() << ":"     << endl;
  os << "  longitude (rad): " << pos.getValue().getLong("rad") << endl;
  os << "  latitude (rad):  " << pos.getValue().getLat("rad")  << endl;
  os << "  height (m):      " << pos.getValue().getLength("m") << endl;
}

int main()
{
  cout << "*** Convert WGS84 to ITRF ***" << endl;

  try {
    MPosition::Convert conv(MPosition::WGS84, MPosition::ITRF);
    while (true) {
      MPosition pos = readPos();
      pos = conv(pos);
      showPos(pos);
    }
  } catch (exception& e) {
    cerr << "Exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}
