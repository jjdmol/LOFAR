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

#if(HAVE_VDM)
// Must be included first. Has a member "signals", which is also a
// macro defined in QT :-)
#include <OCTOPUSSY/Dispatcher.h> 
#include <OCTOPUSSY/Gateways.h>
#include <OCTOPUSSY/Octopussy.h>
#endif // HAVE_VDM



#include <UVPMainWindow.h>


#include <qapplication.h>       // qApp
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qlayout.h>

#include <sstream>              // std::ostringstream

#include <aips/aips.h>
#include <aips/Exceptions.h>
#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/MeasurementSets/MSMainColumns.h>
#include <aips/MeasurementSets/MSPolColumns.h>
#include <aips/Arrays/Vector.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/TableColumn.h>
#include <aips/Quanta/MVTime.h>


#if(HAVE_VDM)
// For VDM stuff
#include <VisAgent/InputAgent.h>
#include <OctoAgent/EventMultiplexer.h>
#include <AID-uvplot.h>
using namespace VisAgent;
#include <UVPOpenVDMDialog.h>
#endif // HAVE_VDM



#if(DEBUG_MODE)
InitDebugContext(UVPMainWindow, "DEBUG_CONTEXT");
#endif

//===================>>>  UVPMainWindow::UVPMainWindow  <<<===================

UVPMainWindow::UVPMainWindow(const std::string& hiidPrefix,
                             bool               kickstartVDM)
#if(HAVE_VDM)
  : QMainWindow(),
    itsInputAgent(0)
#else
  : QMainWindow()
#endif
{

#if(HAVE_VDM)
  itsHIIDPrefix = HIID(hiidPrefix);
  itsEventMultiplexer = new OctoAgent::EventMultiplexer(AidVisualizerWP);
  Octopussy::dispatcher().attach(itsEventMultiplexer,DMI::WRITE);
#endif

  // Construct a menu
  buildMenuBar();
  // Menu constructed


  buildStatusBar();


  itsInputType     = NoInput;
  itsInputFilename = "";
  itsCurrentWorkingDirectory= ".";
  itsMSColumnName  = "DATA";

  itsBusyPlotting = false;

  itsNumberOfChannels  = 0;
  itsNumberOfTimeslots = 0;

  QWidget  *CentralWidget = new QWidget(this);
  setCentralWidget(CentralWidget);
  itsGraphSettingsWidget = new UVPGraphSettingsWidget(30, 1, CentralWidget);

  itsScrollView        = new QScrollView(CentralWidget);
  itsCanvas            = new UVPTimeFrequencyPlot(itsScrollView->viewport());

  QHBoxLayout *hLayout = new QHBoxLayout(CentralWidget);
  hLayout->addWidget(itsScrollView, 5);
  hLayout->addWidget(itsGraphSettingsWidget, 1);

  itsScrollView->addChild(itsCanvas);
  CentralWidget->show();
  hLayout->activate();

  connect(itsCanvas, SIGNAL(signal_mouseWorldPosChanged(double, double)),
          this, SLOT(slot_mouse_world_pos(double, double)));

  connect(itsGraphSettingsWidget, SIGNAL(signalAntenna2Changed(unsigned int)),
          this, SLOT(slot_redraw()));

  connect(itsGraphSettingsWidget, SIGNAL(signalCorrelationChanged(UVPDataAtomHeader::Correlation)),
          this, SLOT(slot_redraw()));

  connect(itsGraphSettingsWidget, SIGNAL(signalFieldsChanged()),
          this, SLOT(slot_redraw()));

  connect(itsGraphSettingsWidget, SIGNAL(signalSpectralWindowChanged(unsigned int)),
          this, SLOT(slot_redraw()));

  connect(itsGraphSettingsWidget, SIGNAL(signalLoadButtonClicked()),
          this, SLOT(slot_loadData()));

  connect(itsCanvas, SIGNAL(signal_timeChanged(double)),
          this, SLOT(slot_setTime(double)));

  connect(itsCanvas, SIGNAL(signal_visibilityChanged(UVPDataAtom::ComplexType)),
          this, SLOT(slot_setVisibility(UVPDataAtom::ComplexType)));

  resizeEvent(0);

  itsCanvas->setXAxis(UVPAxis(1,1,"Channel", ""));
  itsCanvas->setYAxis(UVPAxis(1,1,"Timeslot", ""));
  itsCanvas->drawView();

  setFocusPolicy(QWidget::StrongFocus);

  updateCaption();
#if(HAVE_VDM)
  if(kickstartVDM) {
    itsInputType     = VDM;
    slot_vdmInit(itsHIIDPrefix);
    updateCaption();
    slot_vdmInput();
  }
#endif
}






//==================>>>  UVPMainWindow::~UVPMainWindow  <<<==================

UVPMainWindow::~UVPMainWindow()
{
#if(HAVE_VDM)
  if(itsInputAgent != 0) {
    delete itsInputAgent;
  }
  delete itsEventMultiplexer;
#endif
}





//===============>>>  UVPMainWindow::buildMenuBar  <<<===============

void UVPMainWindow::buildMenuBar()
{
  itsApplicationMenu = new QPopupMenu;
  itsApplicationMenu->insertItem("&Quit", qApp, SLOT(quit()));

  itsDatasourceMenu = new QPopupMenu;
  itsDatasourceMenu->insertItem("&Open MS", this, SLOT(slot_openMS()));
#if(HAVE_VDM)
  itsDatasourceMenu->insertItem("&VDM MS", this, SLOT(slot_vdmOpenMS()));
  itsDatasourceMenu->insertItem("&VDM pipeline", this, SLOT(slot_vdmOpenPipe()));
#endif

  itsSettingsMenu = new QPopupMenu;
  itsSettingsMenu->insertItem("Number of antennae", this, SLOT(slot_changeNumberOfAntennae()));
  

  itsHelpMenu = new QPopupMenu;
  itsHelpMenu->insertItem("&About uvplot", this, SLOT(slot_about_uvplot()));

  itsMenuBar = new QMenuBar(this);
  itsMenuBar->insertItem("&Application", itsApplicationMenu);
  itsMenuBar->insertItem("&Data source", itsDatasourceMenu);
  itsMenuBar->insertItem("&Settings", itsSettingsMenu);
  //  itsMenuBar->insertItem("&Process control", itsProcessControlMenu);
  itsMenuBar->insertSeparator();
  itsMenuBar->insertItem("A&bout", itsHelpMenu);
}







//===============>>>  UVPMainWindow::buildStatusBar  <<<===============

void UVPMainWindow::buildStatusBar()
{
  itsStatusBar       = new QStatusBar(this);
  itsProgressBar     = new QProgressBar(itsStatusBar);
  itsXPosLabel       = new QLabel(itsStatusBar);
  itsYPosLabel       = new QLabel(itsStatusBar);
  itsTimeLabel       = new QLabel(itsStatusBar);
  itsVisibilityLabel = new QLabel(itsStatusBar);

  itsStatusBar->addWidget(itsXPosLabel      , 2, true);
  itsStatusBar->addWidget(itsYPosLabel      , 2, true);
  itsStatusBar->addWidget(itsTimeLabel      , 2, true);
  itsStatusBar->addWidget(itsVisibilityLabel, 2, true);
  itsStatusBar->addWidget(itsProgressBar    , 7, true);
  
  itsStatusBar->show();
}





//===============>>>  UVPMainWindow::updateCaption  <<<===============

void UVPMainWindow::updateCaption()
{
  std::ostringstream out;
  
  out << "Uvplot - ";

  switch(itsInputType) {
  case NoInput:
    {
      out << "No input";
    }
    break;

#if(HAVE_VDM)
  case VDM:
    {
      out << "VDM: ";
    }
    break;
#endif    

  case MS:
    {
      out << "MS: ";
    }
    break;
    
  case PVD:
    {
      out << "PVD: ";
    }
    break;
  }

  out << itsInputFilename;
  
  QString Caption(out.str().c_str());
  setCaption(Caption);
}






//==================>>>  Uvpmainwindow::resizeEvent  <<<==================

void UVPMainWindow::resizeEvent(QResizeEvent */*event*/)
{
  QWidget* CentralWidget = this->centralWidget();
  itsScrollView->setGeometry(0, 0, CentralWidget->width() - itsGraphSettingsWidget->width(), CentralWidget->height());
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
}






//==================>>>  UVPMainWindow::keyPressEvent  <<<==================

void UVPMainWindow::keyPressEvent(QKeyEvent* event)
{
  switch(event->key()) {
  case Key_Up:
    {
      itsScrollView->scrollBy(0,-40);
      event->accept();
    }
  break;

  case Key_Down:
    {
      itsScrollView->scrollBy(0,40);
      event->accept();
    }
  break;

  default:
    {
      event->ignore();
    }
    break;
  }
}






//====================>>>  UVPMainWindow::drawDataSet  <<<====================

void UVPMainWindow::drawDataSet()
{
  // Make sure that there is any data to plot at all
 if(itsDataSet.size() == 0) {
    return;
  }

  unsigned int spectraAdded = 0;

  unsigned int ant1 = itsGraphSettingsWidget->getSettings().getAntenna1();
  unsigned int ant2 = itsGraphSettingsWidget->getSettings().getAntenna2();

  UVPDataAtomHeader FromHeader(ant1, ant2);
  UVPDataAtomHeader ToHeader(FromHeader);
  
  ToHeader.itsAntenna2 = ToHeader.itsAntenna2 + 1;

  itsCanvas->setChannels(itsNumberOfChannels); // Number of channels. Clears buffer.
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);


  UVPDataSet::iterator EndOfData = itsDataSet.end();
  UVPDataSet::iterator Start = itsDataSet.upper_bound(FromHeader);

  if(Start->second.getHeader().itsAntenna1 != ant1 ||
     Start->second.getHeader().itsAntenna2 != ant2) {
    FromHeader.itsAntenna1 = ant2;
    FromHeader.itsAntenna2 = ant1;
    ToHeader = FromHeader;
    ToHeader.itsAntenna2++;
  }
  Start = itsDataSet.upper_bound(FromHeader);

  UVPDataSet::iterator EndOfRecords = itsDataSet.upper_bound(ToHeader);

  
  UVPDataAtomHeader::Correlation Correlation = itsGraphSettingsWidget->getSettings().getCorrelation();

  unsigned int  SpectralWindowID = itsGraphSettingsWidget->getSettings().getSpectralWindow();


  for(UVPDataSet::iterator p = Start;
      p != EndOfRecords && p != EndOfData; p++) {
    
    const   UVPDataAtom *dataAtom = &(p->second);
    
    //********** Data are only added if correlation type is right ********
    if(dataAtom->getHeader().itsCorrelationType  == Correlation &&
       dataAtom->getHeader().itsSpectralWindowID == SpectralWindowID &&
       itsGraphSettingsWidget->getSettings().mustPlotField(dataAtom->getHeader().itsFieldID)) {

      unsigned int NumChan = dataAtom->getNumberOfChannels();
      //      double*      Values  = new double[NumChan];
      //      const UVPDataAtom::ComplexType* data = dataAtom->getData(0);

      //      for(unsigned int j = 0; j < NumChan; j++) {
      //Values[j] = std::abs(*data++);
      //}
      
      //      UVPSpectrum Spectrum(NumChan, spectraAdded, Values);
      
      //      itsCanvas->slot_addSpectrum(Spectrum);
      itsCanvas->slot_addDataAtom(dataAtom);
      spectraAdded++;
      //      delete[] Values;
    }
  }
  itsCanvas->drawView();
}




//================>>>  UVPMainWindow::slot_redraw  <<<================

void UVPMainWindow::slot_redraw()
{
  drawDataSet();
}



//=================>>>  UVPMainWindow::slot_about_uvplot  <<<=================

void UVPMainWindow::slot_about_uvplot()
{
  QMessageBox::information(this, "About uvplot",
                           "UV data visualizer for the LOFAR project\n"
                           "by Michiel Brentjens (brentjens@astron.nl)");
}






//===============>>>  UVPMainWindow::slot_mouse_world_pos  <<<===============

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




//=============>>>  UVPMainWindow::slot_setProgressTotalSteps <<<=============

void UVPMainWindow::slot_setProgressTotalSteps(int steps)
{
  itsProgressBar->setTotalSteps(steps);
}






//==================>>>  UVPMainWindow::slot_setProgress <<<==================

void UVPMainWindow::slot_setProgress(int steps)
{
  itsProgressBar->setProgress(steps);
}





//=================>>>  UVPMainWindow::slot_quitPlotting  <<<=================

void UVPMainWindow::slot_quitPlotting()
{
  itsBusyPlotting = false;
}








//===========>>>  UVPMainWindow::slot_changeNumberOfAntennae  <<<===========

void UVPMainWindow::slot_changeNumberOfAntennae()
{
  bool ok(false);
  int stations = QInputDialog::getInteger("uvplot: settings", "Specify number of stations", 30, 2,1000,1, &ok, this);
  
  if(stations > 1 && ok) {
    itsGraphSettingsWidget->setNumberOfAntennae(stations);
  }
}








//===============>>>  UVPMainWindow::slot_loadData  <<<===============

void UVPMainWindow::slot_loadData()
{
  switch(itsInputType) {
  case MS:
    {
      slot_readMeasurementSet(itsInputFilename);
    }
    break;
    
#if(HAVE_VDM)
  case VDM:
    {
      slot_vdmInput();
    }
    break;
#endif


  default:
    {
    }
    break;
  }
}





//==================>>>  UVPMainWindow::slot_openMS  <<<==================

void UVPMainWindow::slot_openMS()
try
{
  QString filename = QFileDialog::getExistingDirectory(itsCurrentWorkingDirectory.c_str(),
                                                       this, 
                                                       "", 
                                                       "Open Measurement Set");
  
  if(!filename.isNull()) {
    itsInputFilename           = filename.latin1();
    itsInputFilename           = itsInputFilename.substr(0, itsInputFilename.find_last_of("/"));
    itsCurrentWorkingDirectory = itsInputFilename.substr(0, itsInputFilename.find_last_of("/"));
    itsInputType     = MS;
    updateCaption();

    MeasurementSet    ms(filename.latin1());
    MSAntenna         AntennaTable(ms.antenna());
    MSField           FieldTable(ms.field());
    MSDataDescription DataDescTable(ms.dataDescription());

    itsGraphSettingsWidget->setNumberOfAntennae(AntennaTable.nrow());
    itsGraphSettingsWidget->setNumberOfSpectralWindows(DataDescTable.nrow());

    //itsGraphSettingsWidget->setNumberOfFields(FieldTable.nrow());
#if(DEBUG_MODE)
    std::cout << __FUNCTION__ << ":Fields: " << FieldTable.nrow() << std::endl;
#endif
  }
}
catch(AipsError &err)
{
  QMessageBox::critical(0, "Aips++", (const char*)err.getMesg().c_str(), QMessageBox::Ok|QMessageBox::Default,QMessageBox::NoButton);
}
catch(...)
{
  std::cerr << "slot_openMS: Unhandled exception caught." << std::endl << std::flush;
}




#if(HAVE_VDM)
//==================>>>  UVPMainWindow::slot_vdmOpenMS  <<<==================

void UVPMainWindow::slot_vdmOpenMS()
try
{
  QString filename = QFileDialog::getExistingDirectory(".",
                                                       this, 
                                                       "", 
                                                       "Open Measurement Set");
  if(!filename.isNull()) {
    itsInputFilename = filename.latin1();
    itsInputFilename           = itsInputFilename.substr(0, itsInputFilename.find_last_of("/"));
    itsCurrentWorkingDirectory = itsInputFilename.substr(0, itsInputFilename.find_last_of("/"));
    itsInputType     = VDM;
    updateCaption();

    MeasurementSet ms(filename.latin1());
    MSAntenna      AntennaTable(ms.antenna());
    itsGraphSettingsWidget->setNumberOfAntennae(AntennaTable.nrow());

    slot_vdmInit();

  }
}
catch(AipsError &err)
{
  QMessageBox::critical(0, "Aips++", (const char*)err.getMesg().c_str(), QMessageBox::Ok|QMessageBox::Default,QMessageBox::NoButton);
}
catch(...)
{
  std::cerr << "slot_openMS: Unhandled exception caught." << std::endl << std::flush;
}






//==================>>>  UVPMainWindow::slot_vdmOpenPipe  <<<==================

void UVPMainWindow::slot_vdmOpenPipe()
{
  UVPOpenVDMDialog dlg("Uvplot", this);
  bool ok = dlg.exec();
  
  try {
    if(ok) {
      HIID header(dlg.getHeaderText());
    
      slot_vdmInit(header);
    }
  }
  catch(...){
    QMessageBox::critical(0, "uvplot: VDM",
			  "Invalid HIID, connection failed.",
			  QMessageBox::Ok|QMessageBox::Default,
			  QMessageBox::NoButton);
  }
}






//===============>>>  UVPMainWindow::slot_vdmInit  <<<===============

void UVPMainWindow::slot_vdmInit(const HIID& hiidPrefix)
try
{
  using namespace VisAgent;
  using namespace OctoAgent;
  
  itsMSColumnName = itsGraphSettingsWidget->getSettings().getColumnName();
  int ant1        = int(itsGraphSettingsWidget->getSettings().getAntenna1());

  ostringstream select_sstream;
  select_sstream << "ANTENNA1 = " << ant1 << " || ANTENNA2 = " << ant1;

  // initialize parameter record

  DataRecord::Ref dataref;
  DataRecord& rec = dataref <<= new DataRecord;
 
  rec[FThrowError] = True;
 
  DataRecord& args = rec[AidInput] <<= new DataRecord;

  rec[AidInput][FEventMapIn] <<= new DataRecord;
  rec[AidInput][FEventMapIn][FDefaultPrefix] = hiidPrefix;

  //  args[FMSName]         = itsInputFilename;
  //  args[FDataColumnName] = itsMSColumnName;
  //  args[FTileSize]       = 10;

  // setup selection
  //  DataRecord &select = args[FSelection] <<= new DataRecord;

  //  select[FDDID]              = 0;
  //  select[FFieldIndex]        = 0;
  //  select[FChannelStartIndex] = 0;
  //  select[FChannelEndIndex]   = 127;
  //  select[FSelectionString]   = select_sstream.str();


  itsNumberOfChannels = 128;
  itsNumberOfTimeslots = 1500;

  itsScrollView->removeChild(itsCanvas);
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
  itsScrollView->addChild(itsCanvas);
  resizeEvent(0);

  
  // create agent
  if(itsInputAgent == 0) {
#if(DEBUG_MODE)
    std::cout << "itsInputAgent   = new MSInputAgent(AidInput);"<<std::endl;
#endif
    itsInputAgent   = new VisAgent::InputAgent(itsEventMultiplexer->newSink());
  }

#if(DEBUG_MODE)
  std::cout << args.sdebug(2) << std::endl;
#endif  
  bool res = itsInputAgent->init(rec);

  if( !res ){
    cout<<"agent state: "<<itsInputAgent->stateString()<<endl;
    cout<<"init has failed, exiting...\n";
    delete itsInputAgent;
    itsInputAgent = 0;
    return;
  }
}
catch(AipsError &err)
{
  QMessageBox::critical(0, "Aips++", (const char*)err.getMesg().c_str(), QMessageBox::Ok|QMessageBox::Default,QMessageBox::NoButton);
}
catch(...)
{
  std::cerr << "slot_vdmInit(): Unhandled exception caught." << std::endl << std::flush;
}




//===============>>>  UVPMainWindow::slot_vdmInput  <<<===============

void UVPMainWindow::slot_vdmInput()
try
{
  using namespace std;
  
  if(!itsBusyPlotting) {
    itsBusyPlotting = true;
    if(itsInputAgent != 0) {
      itsDataSet.clear();
      itsNumberOfChannels = 0;
    
      DataRecord::Ref header;
      VisTile::Ref    tile;
      DataRecord::Ref footer;


      UVPDataAtomHeader uvp_header;
      int draw_list = 0;

      int intype = 0;
      while( intype > 0 && itsBusyPlotting) {
        HIID   id;
        ObjRef ref;
        
        intype = itsInputAgent->hasNext();

        if(intype <= 0) {
          std::cout << "intype: " << intype << std::endl;
        } else {
          
          switch(intype){
          case HEADER:
            {
              std::cout << "Ignoring HEADER " << id.toString() << std::endl;
              itsInputAgent->getHeader(header);
            }
            break;

          case DATA:
            {
              itsInputAgent->getNextTile(tile);
              int ncorr = tile.deref().ncorr();
              int nfreq = tile.deref().nfreq();
              int ntime = tile.deref().ntime();
              int ant1  = tile.deref().antenna1();
              int ant2  = tile.deref().antenna2();
        
              if(nfreq > itsNumberOfChannels) {
                itsNumberOfChannels = nfreq;
              }

              if(ncorr > 0 && nfreq > 0 && ntime > 0) {
                uvp_header.itsTime             = 0;
                uvp_header.itsAntenna1         = ant1;
                uvp_header.itsAntenna2         = ant2;
                uvp_header.itsExposureTime     = 0;
                uvp_header.itsFieldID          = 1;
                uvp_header.itsSpectralWindowID = 0;
          
                UVPDataAtom atom(nfreq, uvp_header);
          
                for(VisTile::const_iterator iter=tile.deref().begin();
                    iter != tile.deref().end();
                    iter.next()) {
                  uvp_header.itsTime           = iter.time();
            
                  for(int corr = 0; corr < ncorr && uvp_header.itsTime != 0;corr++) {
                    uvp_header.itsCorrelationType = UVPDataAtomHeader::Correlation(corr+1);
              
                    LoVec_fcomplex data(iter.f_data(corr));
                    LoVec_int      flags(iter.f_flags(corr));
              
              
                    atom.setHeader(uvp_header);
                    atom.setData(data);
                    atom.setFlags(flags);
              
                    itsDataSet[uvp_header] = atom;
                  } 
                }
                if(draw_list % 30 == 0) {
                  drawDataSet();
                }
                draw_list++;
                qApp->processEvents();
              }
            }
            break;
            
          case FOOTER:
            {
              std::cout << "Ignoring FOOTER " << id.toString() << std::endl;
              itsInputAgent->getFooter(footer);
            }
            break;
            
          default:
            {
              std::cout << "ERROR: " << intype << std::endl;
            }
            break;
          }//switch intype
          
        } //if intype...
        
      }//while state > 0...
      
      itsInputAgent->close();
      drawDataSet();
      
      delete itsInputAgent;
      itsInputAgent   = 0;
    } else {
      QMessageBox::information(0, "Uvplot", 
                               "No VisInputAgent. First initialize agent.",
                               QMessageBox::Ok|QMessageBox::Default);
    }
    itsBusyPlotting = false;
  }//itsBusyPlotting
}
catch(AipsError &err)
{
  QMessageBox::critical(0, "Aips++", (const char*)err.getMesg().c_str(), QMessageBox::Ok|QMessageBox::Default,QMessageBox::NoButton);
}
catch(...)
{
  std::cerr << "slot_vdmInput(): Unhandled exception caught." << std::endl << std::flush;
}
#endif // HAVE_VDM






//==============>>>  UVPMainWindow::slot_readMeasurementSet  <<<==============

void UVPMainWindow::slot_readMeasurementSet(const std::string& msName)
try
{
  itsDataSet.clear();

  MeasurementSet ms(msName);
  MSField        FieldTable(ms.field());

  Int ant1        = Int(itsGraphSettingsWidget->getSettings().getAntenna1());
  Int ant2        = Int(itsGraphSettingsWidget->getSettings().getAntenna2());
  itsMSColumnName = itsGraphSettingsWidget->getSettings().getColumnName();

#if(DEBUG_MODE)
  TRACER1("Ant1: " << ant1);
  TRACER1("Ant2: " << ant2);
#endif


  Table        msTable(msName);
  Table        Selection(msTable(msTable.col("ANTENNA1") == ant1 ||
                                 msTable.col("ANTENNA2") == ant1) );

  if(Selection.nrow() == 0) {
    QMessageBox::information(0, "Information", "Selection contains no data",
                             QMessageBox::Ok|QMessageBox::Default);
    return;
  }

  ROArrayColumn<Complex> DataColumn    (Selection, itsMSColumnName);
  ROScalarColumn<Double> TimeColumn    (Selection, "TIME");
  ROScalarColumn<Int>    Antenna1Column(Selection, "ANTENNA1");
  ROScalarColumn<Int>    Antenna2Column(Selection, "ANTENNA2");
  ROScalarColumn<Int>    DataDescColumn(Selection, "DATA_DESC_ID");
  ROScalarColumn<Int>    FieldColumn   (Selection, "FIELD_ID");
  ROArrayColumn<Bool>    FlagColumn    (Selection, "FLAG");
  ROScalarColumn<Double> ExposureColumn(Selection, "EXPOSURE");
  ROArrayColumn<Double>  UVWColumn     (Selection, "UVW");

  MSPolarization           PolarizationTable(ms.polarization());
  ROArrayColumn<Int>       PolTypeColumn(PolarizationTable, "CORR_TYPE");

#if(DEBUG_MODE)
  std::cout << __FUNCTION__<< ": " << Selection.nrow() << " rows selected." << std::endl;
#endif

  unsigned int NumPolarizations = DataColumn(0).shape()[0];
  unsigned int NumChannels      = DataColumn(0).shape()[1];
  unsigned int NumSelected      = Selection.nrow();

  std::vector<int>         PolType(NumPolarizations);
  std::vector<UVPDataAtom>       Atoms(NumPolarizations);
  std::vector<UVPDataAtomHeader> Headers(NumPolarizations);
  
  for(unsigned int i = 0; i < NumPolarizations; i++) {
    IPosition Pos(1,0);
    Pos[0] = i;
    PolType[i] = PolTypeColumn(0)(Pos);
    Headers[i] = UVPDataAtomHeader(ant1, ant2);
    Headers[i].itsCorrelationType = UVPDataAtomHeader::Correlation(PolType[i]);
    Atoms[i]   = UVPDataAtom(NumChannels, Headers[i]);
    Atoms[i].setHeader(Headers[i]);
  }


  itsNumberOfChannels  = NumChannels;
  itsNumberOfTimeslots = 1500;
  
  itsScrollView->removeChild(itsCanvas);
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
  itsScrollView->addChild(itsCanvas);
  resizeEvent(0);

  slot_setProgressTotalSteps(NumSelected);
  
  itsBusyPlotting = true;
  
  for(unsigned int i = 0; i < NumSelected && itsBusyPlotting; i++) {
    
    bool           DeleteData;
    Array<Complex> DataArray(DataColumn(i)); // IMPORTANT, prevents "Data" pointer from dangling!!!
    const Complex* Data = DataArray.getStorage(DeleteData);

    bool           DeleteFlag;
    Array<Bool>    FlagArray(FlagColumn(i));
    const bool*    Flag = FlagArray.getStorage(DeleteFlag);
    
    // ******* LOOK AT THIS LOOP VERY CAREFULLY
    for(unsigned int j = 0; j < NumChannels; j++) {
      for(unsigned int k = 0; k < NumPolarizations; k++) {      
        Atoms[k].setData(j, *Data++);
        Atoms[k].setFlag(j, *Flag++);
      }
    }

    for(unsigned int k = 0; k < NumPolarizations; k++) {
      Headers[k].itsTime             = TimeColumn(i);
      Headers[k].itsAntenna1         = Antenna1Column(i);
      Headers[k].itsAntenna2         = Antenna2Column(i);
      Headers[k].itsExposureTime     = ExposureColumn(i);
      Headers[k].itsFieldID          = FieldColumn(i);
      Headers[k].itsSpectralWindowID = DataDescColumn(i); 
/*Very simple. Should in fact be the value of the DataDescColumn(i)-th
  row of the SPECTRAL_WINDOW_ID column of the data description
  subtable.
*/

      Array<Double> UVWArray;
      bool          DeleteUVW;
      UVWColumn.get(i, UVWArray, true);
      const Double* UVW = UVWArray.getStorage(DeleteUVW);
      Headers[k].itsUVW[0]       = UVW[0];
      Headers[k].itsUVW[1]       = UVW[1];
      Headers[k].itsUVW[2]       = UVW[2];
      UVWArray.freeStorage(UVW, DeleteUVW);
      Atoms[k].setHeader(Headers[k]);
      itsDataSet[Headers[k]]= Atoms[k];
    }      

    DataArray.freeStorage(Data, DeleteData); 
    FlagArray.freeStorage(Flag, DeleteFlag);

    if(i % 200 == 0) {
      slot_setProgress(i+1);
      drawDataSet();
    }
    if(i % 20 == 0) {
      qApp->processEvents();
    }
  }
  itsBusyPlotting = false;

  drawDataSet();  
}
catch(AipsError &err)
{
  QMessageBox::critical(0, "Aips++", (const char*)err.getMesg().c_str(), QMessageBox::Ok|QMessageBox::Default,QMessageBox::NoButton);
}
catch(...)
{
  std::cerr << "slot_readMeasurementSet: Unhandled exception caught." << std::endl << std::flush;
}







//====================>>>  UVPMainWindow::slot_setTime  <<<====================

void UVPMainWindow::slot_setTime(double time /* in MJD seconds*/)
{
  time/= 3600.0*24.0;

  MVTime Time(time);            // Expects time in MJD days
  std::ostringstream out;
  
  if(time != 0) {
    Time.print(out, MVTime::YMD);
  }
  itsTimeLabel->setText(out.str().c_str());
}




//=================>>>  UVPMainWindow::slot_setVisibility  <<<=================

void UVPMainWindow::slot_setVisibility(UVPDataAtom::ComplexType vis)
{
  std::ostringstream out;
  out << vis;
  itsVisibilityLabel->setText(out.str().c_str());
}
