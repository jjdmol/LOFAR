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


  itsStatusBar   = new QStatusBar(this);
  itsProgressBar = new QProgressBar(itsStatusBar);
  itsXPosLabel   = new QLabel(itsStatusBar);
  itsYPosLabel   = new QLabel(itsStatusBar);

  itsStatusBar->addWidget(itsXPosLabel, 2, true);
  itsStatusBar->addWidget(itsYPosLabel, 2, true);
  itsStatusBar->addWidget(itsProgressBar, 5, true);
  
  itsStatusBar->show();

  // Small example view

  itsCube = new UVPImageCube(500, 800);

  // End small example view

  itsCanvas = new UVPUVCoverageArea(this, itsCube);
  itsCanvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height() -itsStatusBar->height());
  itsCanvas->show();

  connect(itsCanvas, SIGNAL(signal_mouseWorldPosChanged(double, double)),
          this, SLOT(slot_mouse_world_pos(double, double)));

  // Update itsCube
  slot_setProgressTotalSteps(500);

  for(int x = 0; x < 500; x++) {
    slot_setProgress(x+1);
    for(int y = 0; y < 800; y++) {
      itsCube->getPixel(x, y)->addPointUniform(sin(double(x)*double(y)/30.0), x*y);
    }
  }
  itsCanvas->drawView();
  // End update itsCube
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

  x_out << itsCanvas->getXAxis()->getType()<< ": " << x
        << " " << itsCanvas->getXAxis()->getUnit();

  y_out << itsCanvas->getYAxis()->getType()<< ": " << y
        << " " << itsCanvas->getYAxis()->getUnit();


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
