#include <UVPMainWindow.h>


#include <qapplication.h>       // qApp
#include <qmessagebox.h>


#include <sstream>              // std::ostringstream

//====================>>>  Tmain_window::Tmain_window  <<<====================

Tmain_window::Tmain_window():QMainWindow()
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


Tmain_window::~Tmain_window()
{
  itsCanvas->setData(0);
  delete itsCube;
}


//==================>>>  Tmain_window::resizeEvent  <<<==================

void Tmain_window::resizeEvent(QResizeEvent */*event*/)
{
  itsCanvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height()-itsStatusBar->height());
}




//==================>>>  Tmain_window::slot_about_uvplot  <<<==================

void Tmain_window::slot_about_uvplot()
{
  QMessageBox::information(this, "About uvplot",
                           "UV data visualiser for the LOFAR project\n"
                           "by Michiel Brentjens (brentjens@astron.nl)");
}



//====================>>>  Tmain_window::slot_mouse_world_pos  <<<====================

void Tmain_window::slot_mouse_world_pos(double x,
                                        double y)
{
  std::ostringstream x_out;
  std::ostringstream y_out;

  x_out << "X: " << x;
  y_out << "Y: " << y;

  itsXPosLabel->setText(x_out.str().c_str());
  itsYPosLabel->setText(y_out.str().c_str());
}
