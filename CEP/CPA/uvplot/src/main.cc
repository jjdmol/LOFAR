#include <qapplication.h>

#include <aips/Exceptions.h>

#include <uvplot/UVPMainWindow.h>

#include <Common/Debug.h>

#include <OCTOPUSSY/OctopussyConfig.h>

int main(int argc, char *argv[])
try
{
  QApplication app(argc, argv);
  Debug::initLevels(argc, (const char **)argv);       // Initialize debugging
  OctopussyConfig::initGlobal(argc,(const char **)argv);


  UVPMainWindow *mainwin = new UVPMainWindow;
  mainwin->resize(800, 800);

  app.setMainWidget(mainwin);
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
