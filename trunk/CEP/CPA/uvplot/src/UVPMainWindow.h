// Copyright notice should go here

// $ID$

#if !defined(MAINWIN_H)
#define MAINWIN_H


#include <qwidget.h>
#include <qmenubar.h>
#include <qpopupmenu.h>


#include <UVPDisplayArea.h>




class Tmain_window:public QWidget
{
  Q_OBJECT                      // to make the signal/slot mechanism work

 public:                        /* Public part */

  enum e_menu_command{mc_open, mc_quit, mc_help, mc_information};

  
  Tmain_window();

 protected:                     /* Protected part */
  
  QMenuBar*       m_menu_bar;
  QPopupMenu*     m_file_menu;
  QPopupMenu*     m_view_menu;
  QPopupMenu*     m_help_menu;

  UVPDisplayArea* m_canvas;      /* The drawing canvas */



  virtual void resizeEvent(QResizeEvent *event);

  protected slots:

    void slot_about_uvplot();
};

#endif // MAINWIN_H
