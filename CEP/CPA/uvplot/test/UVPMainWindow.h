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

#include <UVPTimeFrequencyPlot.h>
#include <UVPGraphSettingsWidget.h>
#include <uvplot/UVPDataSet.h>

//#include <VisAgent/VisInputAgent.h>
class OctoMultiplexer;
class VisInputAgent;
#include <DMI/HIID.h>


#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

#pragma aidgroup uvplot
#pragma aid VisualizerWP


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

  enum InputType{NoInput, VDM, MS, PVD};


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
      
  //  void slot_plotTimeFrequencyImage();

  //! VDM input selected by user
  void slot_vdmInput();

  //! Initialize VDM communications. Called from slot_vdmInput().
  void slot_vdmInit(const HIID& header = HIID(""),
		    const HIID& data   = HIID(""));

  //! Stop VDM
  void slot_quitPlotting();

  //! Lets the user select an MS filename. Sets itsInputType to "MS".
  void slot_openMS();

  //! Lets the user select an MS filename. Sets itsInputType to "VDM".
  void slot_vdmOpenMS();

  void slot_vdmOpenPipe();

  //! Called when "load" button is pressed
  void slot_loadData();

  //! Let user set number of antennae.
  void slot_changeNumberOfAntennae();


  //! Actually reads data from MS msName.
  void slot_readMeasurementSet(const std::string& msName);

 protected:                     /* Protected part */
  
  QMenuBar*       itsMenuBar;
  QPopupMenu*     itsApplicationMenu;
  QPopupMenu*     itsDatasourceMenu;
  QPopupMenu*     itsProcessControlMenu;
  QPopupMenu*     itsSettingsMenu;
  QPopupMenu*     itsHelpMenu;
  
  int             itsMenuPlotImageID;
  int             itsMenuPlotStopID;

  QStatusBar*     itsStatusBar;
  QProgressBar*   itsProgressBar; /* Resides in Status bar */
  QLabel*         itsXPosLabel;
  QLabel*         itsYPosLabel;
  QLabel*         itsTimeLabel;
  QLabel*         itsVisibilityLabel;
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

  VisInputAgent*   itsVisInputAgent;
  OctoMultiplexer* itsOctoMultiplexer;

  virtual void resizeEvent  (QResizeEvent* event);
  virtual void keyPressEvent(QKeyEvent*    event);

 protected slots:
    
  //! Display the world coordinates of the mouse pointer in the statusbar
  void slot_mouse_world_pos(double x,
                            double y);
  
  void slot_about_uvplot();

  void slot_setTime(double time);

  void slot_setVisibility(UVPDataAtom::ComplexType vis);


private:
  
  void buildMenuBar();
  void buildStatusBar();
};

#endif // UVPMAINWINDOW_H
