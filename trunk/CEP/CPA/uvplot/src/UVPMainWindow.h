// Copyright notice should go here

#if !defined(UVPMAINWINDOW_H)
#define UVPMAINWINDOW_H

// $Id$


//#include <qwidget.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qlabel.h>

//#include <UVPUVCoverageArea.h>
#include <UVPTimeFrequencyPlot.h>
#include <UVPGraphSettingsWidget.h>

#include <UVPDataSet.h>


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

  enum e_menu_command{mc_file_open,
                      mc_file_quit,
                      mc_plot_image,
                      mc_plot_stop,
                      mc_help_about};

  enum e_plotDataType{plotAmplitude, plotPhase, plotReal, plotImaginary};

  
   UVPMainWindow();
  ~UVPMainWindow();

  public slots:

  void slot_setProgressTotalSteps(int steps);
  void slot_setProgress(int steps);
      
  void slot_plotTimeFrequencyImage();

  void slot_quitPlotting();

  void slot_openFile();

  void slot_readMeasurementSet(const std::string& msName);

 protected:                     /* Protected part */
  
  QMenuBar*       itsMenuBar;
  QPopupMenu*     itsFileMenu;
  QPopupMenu*     itsPlotMenu;
  QPopupMenu*     itsHelpMenu;
  
  int            itsMenuPlotImageID;
  int            itsMenuPlotStopID;

  QStatusBar*     itsStatusBar;
  QProgressBar*   itsProgressBar; /* Resides in Status bar */
  QLabel*         itsXPosLabel;
  QLabel*         itsYPosLabel;

  UVPTimeFrequencyPlot*  itsCanvas;
  UVPGraphSettingsWidget* itsGraphSettingsWidget;
  bool                   itsBusyPlotting;

  UVPDataSet      itsDataSet;
  //  UVPUVCoverageArea* itsCanvas;      /* The drawing canvas */
  //  UVPImageCube*      itsCube;

  std::vector< std::vector<UVPDataAtom> > itsTestMS;

  virtual void resizeEvent(QResizeEvent *event);

  protected slots:
    
    // Display the world coordinates of the mouse pointer in the statusbar
    void slot_mouse_world_pos(double x,
                              double y);
  
    void slot_about_uvplot();
};

#endif // UVPMAINWINDOW_H
