#include <UVPMainWindow.h>


#include <qapplication.h>       // qApp
#include <qmessagebox.h>

//====================>>>  Tmain_window::Tmain_window  <<<====================

Tmain_window::Tmain_window()
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
  
  // End small example view
  m_canvas = new UVPUVCoverageArea(this, itsCube);
  m_canvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height());
  m_canvas->show();
}


Tmain_window::~Tmain_window()
{
  m_canvas->setData(0);
  delete itsCube;
}


//==================>>>  Tmain_window::resizeEvent  <<<==================

void Tmain_window::resizeEvent(QResizeEvent */*event*/)
{
  m_canvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height());
}




//==================>>>  Tmain_window::slot_about_uvplot  <<<==================

void Tmain_window::slot_about_uvplot()
{
  QMessageBox::information(this, "About uvplot",
                           "UV data visualiser for the LOFAR project\n"
                           "by Michiel Brentjens (brentjens@astron.nl)");
}



