#include <qapplication.h>


#include <UVPMainWindow.h>


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  UVPMainWindow *mainwin = new UVPMainWindow;
  mainwin->resize(800, 800);

  app.setMainWidget(mainwin);
  mainwin->show();
  return app.exec();
}
