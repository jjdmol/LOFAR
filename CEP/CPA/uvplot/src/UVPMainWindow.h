// Copyright notice should go here

// $ID$

#if !defined(UVPMAINWINDOW_H)
#define UVPMAINWINDOW_H


//#include <qwidget.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qlabel.h>

//#include <UVPUVCoverageArea.h>
#include <UVPTimeFrequencyPlot.h>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif


class UVPMainWindow:public QMainWindow
{
  Q_OBJECT                      // to make the signal/slot mechanism work

#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif

 public:                        /* Public part */

  enum e_menu_command{mc_open,
                      mc_quit,
                      mc_help,
                      mc_information};

  enum e_plotDataType{plotAmplitude, plotPhase, plotReal, plotImaginary};

  
   UVPMainWindow();
  ~UVPMainWindow();

 protected:                     /* Protected part */
  
  QMenuBar*       m_menu_bar;
  QPopupMenu*     m_file_menu;
  QPopupMenu*     m_plot_menu;
  QPopupMenu*     m_help_menu;
  
  QStatusBar*     itsStatusBar;
  QProgressBar*   itsProgressBar; /* Resides in Status bar */
  QLabel*         itsXPosLabel;
  QLabel*         itsYPosLabel;

  UVPTimeFrequencyPlot*  itsCanvas;
  //  UVPUVCoverageArea* itsCanvas;      /* The drawing canvas */
  //  UVPImageCube*      itsCube;


  virtual void resizeEvent(QResizeEvent *event);

  public slots:

  void slot_setProgressTotalSteps(int steps);
  void slot_setProgress(int steps);
      
  void slot_plotTimeFrequencyImage();

  protected slots:
    
    // Display the world coordinates of the mouse pointer in the statusbar
    void slot_mouse_world_pos(double x,
                              double y);
  
    void slot_about_uvplot();
};

#endif // UVPMAINWINDOW_H
