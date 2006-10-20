//#  tObjectFactory.cc: Test program for the ObjectFactory class.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include "Shapes.h"
#include "Coordinates.h"
#include <Common/StreamUtil.h>

using namespace LOFAR;

void doShapes()
{
  cout << "Registered classes: "
       << ShapeFactory::instance().registeredClassIds()
       << endl;
  {
    Shape* shape = ShapeFactory::instance().create("Rectangle");
    cout << *shape << endl;
  }
  {
    Shape* shape = ShapeFactory::instance().create("Square");
    cout << *shape << endl;
  }
  {
    Shape* shape = ShapeFactory::instance().create("Ellipse");
    cout << *shape << endl;
  }
  {
    Shape* shape = ShapeFactory::instance().create("Circle");
    cout << *shape << endl;
  }
}

void doCoordinates()
{
  cout << "Registered classes: "
       << CoordinateFactory::instance().registeredClassIds()
       << endl;
  {
    Coordinate* coord = CoordinateFactory::instance().
      create("Cartesian", 2.4, 3.6, 4.8);
    cout << *coord << endl;
  }
  {
    Coordinate* coord = CoordinateFactory::instance().
      create("Cylindrical", 0.6, 1.2, 2.4);
    cout << *coord << endl;
  }
  {
    Coordinate* coord = CoordinateFactory::instance().
      create("Spherical", 0.3, 0.15, 9.6);
    cout << *coord << endl;
  }
}

int main()
{
  cout << endl << "** Shapes **" << endl;
  doShapes();
  cout << endl << "** Coordinates **" << endl;
  doCoordinates();
  cout << endl;
  return 0;
}
