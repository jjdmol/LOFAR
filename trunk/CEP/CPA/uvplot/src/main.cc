#include <qapplication.h>


#include "mainwin.h"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  Tmain_window *mainwin = new Tmain_window;
  mainwin->resize(800, 800);

  app.setMainWidget(mainwin);
  mainwin->show();
  return app.exec();
}
