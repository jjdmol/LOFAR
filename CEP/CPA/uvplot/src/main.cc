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


#include <qapplication.h>

#include <aips/Exceptions.h>

#include <uvplot/UVPMainWindow.h>

#include <Common/Debug.h>

#include <OCTOPUSSY/OctopussyConfig.h>

int main(int argc, char *argv[])
try
{
  std::cout << "Uvplot main" << std::endl << std::flush;
  QApplication app(argc, argv);
  Debug::initLevels(argc, (const char **)argv);       // Initialize debugging
  OctopussyConfig::initGlobal(argc,(const char **)argv);


  std::cout << "Create mainwindow" << std::endl;
  UVPMainWindow *mainwin = new UVPMainWindow;
  mainwin->resize(800, 800);

  std::cout << "Set Mainwidget" << std::endl;
  app.setMainWidget(mainwin);
  std::cout << "Show" << std::endl;
  mainwin->show();
  return app.exec();
}
catch(AipsError &err)
{
  std::cerr << "Aips++: " << err.getMesg() << std::endl << std::flush;
}
catch(...)
{
  std::cerr << "Unhandled exception caught." << std::endl << std::flush;
}
