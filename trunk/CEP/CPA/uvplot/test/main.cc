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

#include <UVPMainWindow.h>

#include <Common/Debug.h>


#if(HAVE_VDM)
#include <OCTOPUSSY/OctopussyConfig.h>
#endif // HAVE_VDM

#include <unistd.h>
#include <string>


struct TSjob_parameters
{
  std::string HIID_prefix;
  
  TSjob_parameters(unsigned int argc, char *argv[]);
};



/*===============>>>  TSjob_parameters::TSjob_parameters  <<<================*/

TSjob_parameters::TSjob_parameters(unsigned int argc, char *argv[])
{
   const char   *getopt_string = "p:";
   signed char   ch = 0;
   bool          invalid_commandline = false;

   HIID_prefix = "";

   for(ch = getopt(argc, argv, getopt_string);
       ch != -1;
       ch = getopt(argc, argv, getopt_string))
     {
       switch(ch)
         {

         case 'p':
           {
             HIID_prefix = std::string(optarg);
           }
           break;

         } // switch

     } // for
}







int main(int argc, char *argv[])
try
{
  std::cout << "Uvplot main" << std::endl << std::flush;
  QApplication app(argc, argv);
  Debug::initLevels(argc, (const char **)argv);       // Initialize debugging

#if(HAVE_VDM)
  OctopussyConfig::initGlobal(argc,(const char **)argv);
#endif
  TSjob_parameters Job(argc, argv);


  std::cout << "Create mainwindow" << std::endl;
  UVPMainWindow *mainwin;
  if(Job.HIID_prefix == "") {
    mainwin = new UVPMainWindow;
  } else {
    mainwin = new UVPMainWindow(Job.HIID_prefix, true);
  }
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
