// Must be included first. Has a member "signals", which is also a
// macro defined in QT :-)
#include <OCTOPUSSY/Dispatcher.h> 
#include <OCTOPUSSY/Gateways.h>

#include <uvplot/UVPMainWindow.h>
#include <uvplot/UVPDataTransferWP.h>    // Communications class
#include <uvplot/UVPPVDInput.h>


#include <qapplication.h>       // qApp
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qlayout.h>

#include <sstream>              // std::ostringstream

#include <aips/aips.h>
#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/MeasurementSets/MSMainColumns.h>
#include <aips/MeasurementSets/MSPolColumns.h>
#include <aips/Arrays/Vector.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/TableColumn.h>
#include <aips/Quanta/MVTime.h>


#if(DEBUG_MODE)
InitDebugContext(UVPMainWindow, "DEBUG_CONTEXT");
#endif

//===================>>>  UVPMainWindow::UVPMainWindow  <<<===================

UVPMainWindow::UVPMainWindow()
  : QMainWindow()
{
  // Construct a menu
  buildMenuBar();
  // Menu constructed


  buildStatusBar();


  itsInputType     = NoInput;
  itsInputFilename = "";
  
  itsBusyPlotting = false;

  itsNumberOfChannels  = 0;
  itsNumberOfTimeslots = 0;

  QWidget  *CentralWidget = new QWidget(this);
  setCentralWidget(CentralWidget);
  itsGraphSettingsWidget = new UVPGraphSettingsWidget(30, CentralWidget);

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

  connect(itsCanvas, SIGNAL(signal_timeChanged(double)),
          this, SLOT(slot_setTime(double)));

  resizeEvent(0);
  itsCanvas->drawView();

  setFocusPolicy(QWidget::StrongFocus);

  updateCaption();
}






//==================>>>  UVPMainWindow::~UVPMainWindow  <<<==================

UVPMainWindow::~UVPMainWindow()
{
  //  delete itsDataSet;
}





//===============>>>  UVPMainWindow::buildMenuBar  <<<===============

void UVPMainWindow::buildMenuBar()
{
  itsFileMenu = new QPopupMenu;
  itsFileMenu->insertItem("&Open MS", this, SLOT(slot_openMS()));
  itsFileMenu->insertItem("&Open PVD", this, SLOT(slot_openPVD()));
  itsFileMenu->insertItem("&Quit", qApp, SLOT(quit()));

  itsPlotMenu = new QPopupMenu;
  itsMenuPlotImageID = itsPlotMenu->insertItem("&Image", this,
                                               SLOT(slot_plotTimeFrequencyImage()));
  itsMenuPlotStopID  = itsPlotMenu->insertItem("&Stop", this,
                                               SLOT(slot_quitPlotting()));

  itsPlotMenu->setItemEnabled(itsMenuPlotImageID, true);
  itsPlotMenu->setItemEnabled(itsMenuPlotStopID, false);


  itsHelpMenu = new QPopupMenu;
  itsHelpMenu->insertItem("&About uvplot", this, SLOT(slot_about_uvplot()));

  itsMenuBar = new QMenuBar(this);
  itsMenuBar->insertItem("&File", itsFileMenu);
  itsMenuBar->insertItem("&Plot", itsPlotMenu);
  itsMenuBar->insertSeparator();
  itsMenuBar->insertItem("&Help", itsHelpMenu);
}







//===============>>>  UVPMainWindow::buildStatusBar  <<<===============

void UVPMainWindow::buildStatusBar()
{
  itsStatusBar   = new QStatusBar(this);
  itsProgressBar = new QProgressBar(itsStatusBar);
  itsXPosLabel   = new QLabel(itsStatusBar);
  itsYPosLabel   = new QLabel(itsStatusBar);
  itsTimeLabel   = new QLabel(itsStatusBar);

  itsStatusBar->addWidget(itsXPosLabel, 2, true);
  itsStatusBar->addWidget(itsYPosLabel, 2, true);
  itsStatusBar->addWidget(itsTimeLabel, 2, true);
  itsStatusBar->addWidget(itsProgressBar, 5, true);
  
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

  case DMI:
    {
      out << "DMI: ";
    }
    break;
    
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

  unsigned int ant1 = itsGraphSettingsWidget->getSettings().getAntenna1();
  unsigned int ant2 = itsGraphSettingsWidget->getSettings().getAntenna2();

  UVPDataAtomHeader FromHeader(ant1, ant2);
  UVPDataAtomHeader ToHeader(FromHeader);
  
  ToHeader.itsAntenna2 = ToHeader.itsAntenna2 + 1;

  itsCanvas->setChannels(itsNumberOfChannels); // Number of channels. Clears buffer.
  unsigned int spectraAdded = 0;

  UVPDataSet::iterator EndOfRecords = itsDataSet.upper_bound(ToHeader);
  UVPDataSet::iterator EndOfData = itsDataSet.end();
  
  UVPDataAtomHeader::Correlation Correlation = itsGraphSettingsWidget->getSettings().getCorrelation();

#if(DEBUG_MODE)
  TRACER1("Correlation= " << Correlation);
#endif

  for(UVPDataSet::iterator p = itsDataSet.upper_bound(FromHeader);
      p != EndOfRecords && p != EndOfData; p++) {
    
    const   UVPDataAtom *dataAtom = &(p->second);
    
    //********** Data are only added if correlation type is right ********
    if(dataAtom->getHeader().itsCorrelationType == Correlation) {

      unsigned int NumChan = dataAtom->getNumberOfChannels();
      double*      Values  = new double[NumChan];
      const UVPDataAtom::ComplexType* data = dataAtom->getData(0);

      for(unsigned int j = 0; j < NumChan; j++) {
        Values[j] = std::abs(*data++);
      }
      
      UVPSpectrum Spectrum(NumChan, spectraAdded, Values);
      
      itsCanvas->slot_addSpectrum(Spectrum);
      itsCanvas->slot_addDataAtom(dataAtom);
      spectraAdded++;
      delete[] Values;
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




//==================>>>  UVPMainWindow::slot_openMS  <<<==================

void UVPMainWindow::slot_openMS()
{
  QString filename = QFileDialog::getExistingDirectory(".",
                                                       this, 
                                                       "", 
                                                       "Open Measurement Set");
  if(!filename.isNull()) {
    itsInputFilename = filename.latin1();
    itsInputType     = MS;
    updateCaption();
    slot_readMeasurementSet(filename.latin1());
  }
}




//==================>>>  UVPMainWindow::slot_openPVD  <<<==================

void UVPMainWindow::slot_openPVD()
{
  QString filename = QFileDialog::getOpenFileName(".",
                                                  "*.pvd *.PVD",
                                                  this, 
                                                  "", 
                                                  "Open Patch Visibility Database");
  if(!filename.isNull()) {
    itsInputFilename = filename.latin1();
    itsInputType     = PVD;
    updateCaption();
    slot_readPVD(filename.latin1());
  }
}




//==========>>>  UVPMainWindow::slot_plotTimeFrequencyImage  <<<==========

void UVPMainWindow::slot_plotTimeFrequencyImage()
{
  if(!itsBusyPlotting) {
    itsInputFilename = "*** DMI ***";
    itsInputType     = DMI;
    updateCaption();
    
    itsPlotMenu->setItemEnabled(itsMenuPlotImageID, false);
    itsPlotMenu->setItemEnabled(itsMenuPlotStopID, true);
    
    itsDataSet.clear();
        
    Dispatcher    dispatcher;     // Octopussy Message Dispatcher

    // Octopussy
    unsigned int patch = 0;

    // Anonymous counted reference. No need to delete.
    UVPDataTransferWP *transfer = new UVPDataTransferWP(patch, &itsDataSet);
    dispatcher.attach(transfer, DMI::ANON);
    initGateways(dispatcher);     // Octopussy
    
    dispatcher.start();           // Octopussy
    
    itsBusyPlotting = true;

    unsigned int previousSize = 0;

    itsNumberOfTimeslots = 1200;

    
    itsScrollView->removeChild(itsCanvas);
    itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
    itsScrollView->addChild(itsCanvas);
    resizeEvent(0);

    // Acquire data.
    while(itsBusyPlotting) {
      dispatcher.poll(50);
      qApp->processEvents();

      if(itsDataSet.size() > previousSize) {
        previousSize = itsDataSet.size();
        
        itsNumberOfChannels = itsDataSet.begin()->second.getNumberOfChannels();
        drawDataSet();
      }
      
    } // while
    

    dispatcher.stop();
    itsCanvas->drawView();

    itsPlotMenu->setItemEnabled(itsMenuPlotImageID, true);
    itsPlotMenu->setItemEnabled(itsMenuPlotStopID, false);

    itsInputFilename = "";
    itsInputType     = NoInput;

    updateCaption();
  }

}





//==============>>>  UVPMainWindow::slot_readMeasurementSet  <<<==============

void UVPMainWindow::slot_readMeasurementSet(const std::string& msName)
{
  itsDataSet.clear();

  MeasurementSet ms(msName);
  MSAntenna      AntennaTable(ms.antenna());
  MSField        FieldTable(ms.field());

  
  std::cout << "=========>>> Table thing  <<<=========" << std::endl;
  Int ant1 = Int(itsGraphSettingsWidget->getSettings().getAntenna1());
  Int ant2 = Int(itsGraphSettingsWidget->getSettings().getAntenna2());

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

  ROArrayColumn<Complex> DataColumn    (Selection, "DATA");
  ROScalarColumn<Double> TimeColumn    (Selection, "TIME");
  ROScalarColumn<Int>    Antenna1Column(Selection, "ANTENNA1");
  ROScalarColumn<Int>    Antenna2Column(Selection, "ANTENNA2");
  ROScalarColumn<Int>    FieldColumn   (Selection, "FIELD_ID");
  ROArrayColumn<Bool>    FlagColumn    (Selection, "FLAG");
  ROScalarColumn<Double> ExposureColumn(Selection, "EXPOSURE");

  unsigned int NumRows          = ms.nrow();
  unsigned int NumAntennae      = AntennaTable.nrow();
  unsigned int NumBaselines     = NumAntennae*(NumAntennae-1)/2;
  unsigned int NumPolarizations = DataColumn(0).shape()[0];
  unsigned int NumChannels      = DataColumn(0).shape()[1];
  unsigned int NumSelected      = Selection.nrow();

  MSPolarization           PolarizationTable(ms.polarization());
  ROArrayColumn<Int>       PolTypeColumn(PolarizationTable, "CORR_TYPE");

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
    Array<Complex> DataArray(DataColumn(i));
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
      Headers[k].itsTime         = TimeColumn(i);
      Headers[k].itsAntenna1     = Antenna1Column(i);
      Headers[k].itsAntenna2     = Antenna2Column(i);
      Headers[k].itsExposureTime = ExposureColumn(i);
      Headers[k].itsFieldID      = FieldColumn(i);
      Headers[k].sortAntennae();
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

#if(DEBUG_MODE)
  TRACER1("Selection.nrow(); " << Selection.nrow());
  TRACER1("NumRows         : " << NumRows);
  TRACER1("NumBaselines    : " << NumBaselines);
  TRACER1("NumPolarizations: " << NumPolarizations);
#endif

}







//==================>>>  UVPMainWindow::slot_readPVD  <<<==================

void UVPMainWindow::slot_readPVD(const std::string& pvdName)
{
  itsDataSet.clear();
  UVPPVDInput pvd(pvdName);

#if(DEBUG_MODE)
  TRACER1("Num Ant : " << pvd.numberOfAntennae());
  TRACER1("Num Chan: " << pvd.numberOfChannels());
#endif

  itsNumberOfChannels  = pvd.numberOfChannels();


  itsScrollView->removeChild(itsCanvas);
  itsNumberOfTimeslots = 1500;
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
  itsScrollView->addChild(itsCanvas);
  resizeEvent(0);

  unsigned int ant1 = itsGraphSettingsWidget->getSettings().getAntenna1();
  unsigned int ant2 = itsGraphSettingsWidget->getSettings().getAntenna2();

  unsigned int pass(0);

  itsBusyPlotting = true;

  while(pvd.getDataAtoms(&itsDataSet, ant1, ant2) && itsBusyPlotting) {
    if(pass % 20 == 0) {
      drawDataSet();
    }
    pass++;
    qApp->processEvents();
    
  } // while

  itsBusyPlotting = false;
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
