#include <UVPMainWindow.h>


#include <qapplication.h>       // qApp
#include <qmessagebox.h>


#include <sstream>              // std::ostringstream

//====================>>>  UVPMainWindow::UVPMainWindow  <<<====================

UVPMainWindow::UVPMainWindow():QMainWindow()
{
  m_file_menu = new QPopupMenu;
  m_file_menu->insertItem("&Open", mc_open);
  m_file_menu->insertItem("&Quit", qApp, SLOT(quit()));

  m_help_menu = new QPopupMenu;
  m_help_menu->insertItem("&About uvplot", this, SLOT(slot_about_uvplot()));

  m_menu_bar = new QMenuBar(this);
  m_menu_bar->insertItem("&File", m_file_menu);
  m_menu_bar->insertSeparator();
  m_menu_bar->insertItem("&Help", m_help_menu);


  // Small example view

  itsCube = new UVPImageCube(500, 800);
  for(int x = 0; x < 500; x++) {
    for(int y = 0; y < 800; y++) {
      itsCube->getPixel(x, y)->addPointUniform(sin(double(x)*double(y)/30.0), x*y);
    }
  }

  itsStatusBar   = new QStatusBar(this);
  itsProgressBar = new QProgressBar(itsStatusBar);
  itsXPosLabel   = new QLabel(itsStatusBar);
  itsYPosLabel   = new QLabel(itsStatusBar);

  itsStatusBar->addWidget(itsXPosLabel, 2, true);
  itsStatusBar->addWidget(itsYPosLabel, 2, true);
  itsStatusBar->addWidget(itsProgressBar, 5, true);
  
  itsStatusBar->show();

  // End small example view
  itsCanvas = new UVPUVCoverageArea(this, itsCube);
  itsCanvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height() -itsStatusBar->height());
  itsCanvas->show();

  itsProgressBar->setTotalSteps(100);
  itsProgressBar->setProgress(40);

  connect(itsCanvas, SIGNAL(signal_mouse_world_pos_changed(double, double)),
          this, SLOT(slot_mouse_world_pos(double, double)));
}


UVPMainWindow::~UVPMainWindow()
{
  itsCanvas->setData(0);
  delete itsCube;
}


//==================>>>  UVPMainWindow::resizeEvent  <<<==================

void UVPMainWindow::resizeEvent(QResizeEvent */*event*/)
{
  itsCanvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height()-itsStatusBar->height());
}




//==================>>>  UVPMainWindow::slot_about_uvplot  <<<==================

void UVPMainWindow::slot_about_uvplot()
{
  QMessageBox::information(this, "About uvplot",
                           "UV data visualiser for the LOFAR project\n"
                           "by Michiel Brentjens (brentjens@astron.nl)");
}



//====================>>>  UVPMainWindow::slot_mouse_world_pos  <<<====================

void UVPMainWindow::slot_mouse_world_pos(double x,
                                        double y)
{
  std::ostringstream x_out;
  std::ostringstream y_out;

  x_out << "X: " << x;
  y_out << "Y: " << y;

  itsXPosLabel->setText(x_out.str().c_str());
  itsYPosLabel->setText(y_out.str().c_str());
}




//====================>>>UVPMainWindow::slot_setProgressTotalSteps <<<====================

void UVPMainWindow::slot_setProgressTotalSteps(int steps)
{
  itsProgressBar->setTotalSteps(steps);
}






//====================>>>UVPMainWindow::slot_setProgress <<<====================

void UVPMainWindow::slot_setProgress(int steps)
{
  itsProgressBar->setProgress(steps);
}
