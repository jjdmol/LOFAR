//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(UVPMAINWINDOW_H)
#define UVPMAINWINDOW_H

// $Id$


#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qscrollview.h>

#include <uvplot/UVPTimeFrequencyPlot.h>
#include <uvplot/UVPGraphSettingsWidget.h>
#include <uvplot/UVPDataSet.h>


#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif



//! The main window class
/*!
 */
class UVPMainWindow:public QMainWindow
{
  Q_OBJECT                      // to make the signal/slot mechanism work

#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif

 public:                        /* Public part */

  enum e_plotDataType{plotAmplitude, plotPhase, plotReal, plotImaginary};

  enum InputType{NoInput, DMI, MS, PVD};


  //! Constructor
   UVPMainWindow();

  //! Destructor
  ~UVPMainWindow();

  //! Draws data set considering all current settings
  void drawDataSet();

  //! Sets window caption.
  void updateCaption();

  public slots:
  
  //! Only calls drawDataSet
  void slot_redraw();

  void slot_setProgressTotalSteps(int steps);
  void slot_setProgress(int steps);
      
  void slot_plotTimeFrequencyImage();

  void slot_quitPlotting();

  void slot_openMS();
  void slot_openPVD();

  void slot_readMeasurementSet(const std::string& msName);
  void slot_readPVD(const std::string& pvdName);

 protected:                     /* Protected part */
  
  QMenuBar*       itsMenuBar;
  QPopupMenu*     itsFileMenu;
  QPopupMenu*     itsPlotMenu;
  QPopupMenu*     itsHelpMenu;
  
  int             itsMenuPlotImageID;
  int             itsMenuPlotStopID;

  QStatusBar*     itsStatusBar;
  QProgressBar*   itsProgressBar; /* Resides in Status bar */
  QLabel*         itsXPosLabel;
  QLabel*         itsYPosLabel;
  QLabel*         itsTimeLabel;
  QScrollView*    itsScrollView;

  unsigned int    itsNumberOfChannels;
  unsigned int    itsNumberOfTimeslots;

  UVPTimeFrequencyPlot*   itsCanvas;
  UVPGraphSettingsWidget* itsGraphSettingsWidget;
  bool                    itsBusyPlotting;

  UVPDataSet      itsDataSet;

  InputType       itsInputType;
  std::string     itsInputFilename;
  std::string     itsMSColumnName;

  virtual void resizeEvent  (QResizeEvent* event);
  virtual void keyPressEvent(QKeyEvent*    event);

 protected slots:
    
  //! Display the world coordinates of the mouse pointer in the statusbar
  void slot_mouse_world_pos(double x,
                            double y);
  
  void slot_about_uvplot();

  void slot_setTime(double time);


private:
  
  void buildMenuBar();
  void buildStatusBar();
};

#endif // UVPMAINWINDOW_H
