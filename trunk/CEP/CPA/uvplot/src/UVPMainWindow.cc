// Must be included first. Has a member "signals", which is also a
// macro defined in QT :-)
#include <OCTOPUSSY/Dispatcher.h> 
#include <OCTOPUSSY/Gateways.h>
//#include <UVD/MSIntegratorWP.h>

#include <UVPMainWindow.h>
#include <UVPDataTransferWP.h>    // Communications class
#include <UVPPVDInput.h>


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
#include <trial/MeasurementEquations/VisibilityIterator.h>
#include <trial/MeasurementEquations/VisBuffer.h>


#if(DEBUG_MODE)
InitDebugContext(UVPMainWindow, "DEBUG_CONTEXT");
#endif

//===================>>>  UVPMainWindow::UVPMainWindow  <<<===================

UVPMainWindow::UVPMainWindow()
  : QMainWindow()
{
  //  itsDataSet = new UVPDataSet;

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



  itsStatusBar   = new QStatusBar(this);
  itsProgressBar = new QProgressBar(itsStatusBar);
  itsXPosLabel   = new QLabel(itsStatusBar);
  itsYPosLabel   = new QLabel(itsStatusBar);

  itsStatusBar->addWidget(itsXPosLabel, 2, true);
  itsStatusBar->addWidget(itsYPosLabel, 2, true);
  itsStatusBar->addWidget(itsProgressBar, 5, true);
  
  itsStatusBar->show();
  itsBusyPlotting = false;

  itsNumberOfChannels  = 0;
  itsNumberOfTimeslots = 0;

  QWidget  *CentralWidget = new QWidget(this);
  setCentralWidget(CentralWidget);
  itsGraphSettingsWidget = new UVPGraphSettingsWidget(30, CentralWidget);

  itsScrollView = new QScrollView(CentralWidget);
  
  QHBoxLayout *hLayout = new QHBoxLayout(CentralWidget);

#if(DEBUG_MODE)
  TRACER1("itsCanvas = new UVPTimeFrequencyPlot(itsScrollView->viewport());");
#endif
  itsCanvas              = new UVPTimeFrequencyPlot(itsScrollView->viewport());

  hLayout->addWidget(itsScrollView, 5);
  hLayout->addWidget(itsGraphSettingsWidget, 1);

  itsScrollView->addChild(itsCanvas);
  CentralWidget->show();
  hLayout->activate();

  connect(itsCanvas, SIGNAL(signal_mouseWorldPosChanged(double, double)),
          this, SLOT(slot_mouse_world_pos(double, double)));

  resizeEvent(0);
  itsCanvas->drawView();

    // End update itsCube
}






//==================>>>  UVPMainWindow::~UVPMainWindow  <<<==================

UVPMainWindow::~UVPMainWindow()
{
  //  delete itsDataSet;
}





//==================>>>  UVPMainWindow::resizeEvent  <<<==================

void UVPMainWindow::resizeEvent(QResizeEvent */*event*/)
{
  QWidget* CentralWidget = this->centralWidget();
  itsScrollView->setGeometry(0, 0, CentralWidget->width() - itsGraphSettingsWidget->width(), CentralWidget->height());
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
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





//==========>>>  UVPMainWindow::slot_plotTimeFrequencyImage  <<<==========

void UVPMainWindow::slot_plotTimeFrequencyImage()
{
  if(!itsBusyPlotting) {
    
    itsPlotMenu->setItemEnabled(itsMenuPlotImageID, false);
    itsPlotMenu->setItemEnabled(itsMenuPlotStopID, true);
    
    Dispatcher    dispatcher;     // Octopussy Message Dispatcher

    // Octopussy
    unsigned int patch = 0;

    // Anonymous counted reference. No need to delete.
    UVPDataTransferWP *transfer = new UVPDataTransferWP(patch, &itsDataSet);
    dispatcher.attach(transfer, DMI::ANON);
    initGateways(dispatcher);     // Octopussy
    
    dispatcher.start();           // Octopussy
    
    slot_setProgressTotalSteps(600);
    
    itsBusyPlotting = true;

    unsigned int spectraAdded(0);
    unsigned int previousSize = 0;

    UVPDataAtomHeader FromHeader;
    UVPDataAtomHeader ToHeader;

    FromHeader.itsAntenna1 = itsGraphSettingsWidget->getSettings().getAntenna1();
    FromHeader.itsAntenna2 = itsGraphSettingsWidget->getSettings().getAntenna2();;
    
    ToHeader.itsAntenna1 = FromHeader.itsAntenna1;
    ToHeader.itsAntenna2 = FromHeader.itsAntenna2 + 1;
      
    while(itsBusyPlotting) {
      dispatcher.poll(50);
      qApp->processEvents();

      bool doDraw(false);
      
      // First experiment to see how fast I can paint...

      if(itsDataSet.size() > previousSize) {
#if(DEBUG_MODE)
        TRACER1("ItsDataSet.size()" << itsDataSet.size());
#endif
        doDraw = true;
        previousSize = itsDataSet.size();
      }

      if(doDraw) {
        itsNumberOfChannels = itsDataSet.begin()->second.getNumberOfChannels();
        itsCanvas->setChannels(itsDataSet.begin()->second.getNumberOfChannels()); // Number of channels. Clears buffer.
        spectraAdded = 0;
        for(UVPDataSet::iterator p = itsDataSet.upper_bound(FromHeader);
            p != itsDataSet.upper_bound(ToHeader) && p != itsDataSet.end();
            p++) {
          const   UVPDataAtom *dataAtom = &(p->second);
#if(DEBUG_MODE)
          TRACER2( "Ant1: " <<  dataAtom->getHeader().itsAntenna1);
          TRACER2( "Ant2: " <<  dataAtom->getHeader().itsAntenna2);
          cout.precision(10);
          TRACER2( "Time: " <<  dataAtom->getHeader().itsTime); 
          cout.precision(6);
          TRACER2( "Corr: " <<  dataAtom->getHeader().itsCorrelationType);
#endif
          double *Values = new double[dataAtom->getNumberOfChannels()];
          for(unsigned int j = 0; j < dataAtom->getNumberOfChannels(); j++) {
            Values[j] = std::abs(*dataAtom->getData(j));
          }
          UVPSpectrum   Spectrum(dataAtom->getNumberOfChannels(), spectraAdded, Values);
          if(dataAtom->getHeader().itsCorrelationType == UVPDataAtomHeader::RR) {
            itsCanvas->slot_addSpectrum(Spectrum);
            spectraAdded++;
          }
          delete[] Values;
        }
        itsNumberOfTimeslots = spectraAdded;
        itsCanvas->setGeometry(0, 0, itsNumberOfChannels, itsNumberOfTimeslots);
        itsCanvas->drawView();
      }
      
    } // while
    

    dispatcher.stop();
    itsCanvas->drawView();
    slot_setProgress(0);

    itsPlotMenu->setItemEnabled(itsMenuPlotImageID, true);
    itsPlotMenu->setItemEnabled(itsMenuPlotStopID, false);
  }

}





//=================>>>  UVPMainWindow::slo_quitPlotting  <<<=================

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
    
    slot_readPVD(filename.latin1());
  }
}




//==============>>>  UVPMainWindow::slot_readMeasurementSet  <<<==============

void UVPMainWindow::slot_readMeasurementSet(const std::string& msName)
{
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
  Table        Selection(msTable((msTable.col("ANTENNA1") == ant1 &&
                                  msTable.col("ANTENNA2") == ant2) ||
                                 (msTable.col("ANTENNA1") == ant2 &&
                                  msTable.col("ANTENNA2") == ant1) )
                         );

  if(Selection.nrow() == 0) {
    QMessageBox::information(0, "Information", "Selection contains no data",
                             QMessageBox::Ok|QMessageBox::Default);
    return;
  }

  ROArrayColumn<Complex> DataColumn(Selection, "DATA");
  //  ROScalarColumn<Float>  TimeColumn(Selection, "TIME");

  unsigned int NumRows          = ms.nrow();
  unsigned int NumAntennae      = AntennaTable.nrow();
  unsigned int NumBaselines     = NumAntennae*(NumAntennae-1)/2;
  unsigned int NumPolarizations = DataColumn(0).shape()[0];
  unsigned int NumChannels      = DataColumn(0).shape()[1];
  unsigned int NumTimeslots     = Selection.nrow();

  itsNumberOfChannels  = NumChannels;
  itsNumberOfTimeslots = NumTimeslots;
  
  itsCanvas->setChannels(NumChannels);
  itsScrollView->removeChild(itsCanvas);
  itsScrollView->addChild(itsCanvas);
  resizeEvent(0);

  slot_setProgressTotalSteps(NumTimeslots);
  for(unsigned int i = 0; i < NumTimeslots; i++) {
    double*     Values = new double[NumChannels];
    IPosition   Pos(2, 0);
    for(unsigned int j = 0; j < NumChannels; j++) {
      Pos[0] = 0;
      Pos[1] = j;
      Complex     Visibility0 = DataColumn(i)(Pos);
      Pos[0] = 1;
      Complex     Visibility1 = DataColumn(i)(Pos);
      Values[j] = fabs(Visibility0-Visibility1);
    }
    UVPSpectrum   Spectrum(NumChannels, i, Values);
    itsCanvas->slot_addSpectrum(Spectrum);
    delete[] Values;
    slot_setProgress(i+1);
    if(i % 3 == 0) {
      itsCanvas->drawView();
      qApp->processEvents();
    }
  }
  itsCanvas->drawView();
  

  std::cout << DataColumn(0).shape() << std::endl;
  std::cout << "=========>>> Table thing  <<<=========" << std::endl << std::flush;

  
#if(DEBUG_MODE)
  TRACER1("Selection.nrow(); " <<   Selection.nrow());
  TRACER1("NumRows         : " << NumRows);
  TRACER1("NumBaselines    : " << NumBaselines);
  TRACER1("NumPolarizations: " << NumPolarizations);
  TRACER1("NumTimeslots    : " << NumTimeslots << std::flush);
#endif

}







//==================>>>  UVPMainWindow::slot_readPVD  <<<==================

void UVPMainWindow::slot_readPVD(const std::string& pvdName)
{
  UVPPVDInput pvd(pvdName);

#if(DEBUG_MODE)
  TRACER1("Num Ant : " << pvd.numberOfAntennae());
  TRACER1("Num Chan: " << pvd.numberOfChannels());
#endif

  itsNumberOfChannels  = pvd.numberOfChannels();

  itsCanvas->setChannels(itsNumberOfChannels);
  itsCanvas->setGeometry(0, 0, itsNumberOfChannels, 1500);
  itsScrollView->removeChild(itsCanvas);
  itsScrollView->addChild(itsCanvas);
  resizeEvent(0);


  std::vector<UVPDataAtom> DataSlice;

  unsigned int ant1 = itsGraphSettingsWidget->getSettings().getAntenna1();
  unsigned int ant2 = itsGraphSettingsWidget->getSettings().getAntenna2();

  UVPDataAtomHeader FromHeader;
  UVPDataAtomHeader ToHeader;
  
  FromHeader.itsAntenna1 = std::min(ant1, ant2);
  FromHeader.itsAntenna2 = std::max(ant1, ant2);

  ant1 = FromHeader.itsAntenna1;
  ant2 = FromHeader.itsAntenna2;
  
  ToHeader.itsAntenna1 = FromHeader.itsAntenna1;
  ToHeader.itsAntenna2 = FromHeader.itsAntenna2 + 1;
  
  unsigned int pass(0);

  while(pvd.getDataAtoms(&itsDataSet, ant1, ant2)) {
    if(pass % 5 == 0) {
#if(DEBUG_MODE)
      TRACER1( "itsDataSet.size(): " << itsDataSet.size());
      TRACER1( "itsDataSet.begin(): " << itsDataSet.begin()->first.itsAntenna1 << "-" <<
               itsDataSet.begin()->first.itsAntenna2);
      TRACER1( "itsDataSet.end(): " << (itsDataSet.end()-- )->first.itsAntenna1 << "-" <<
               (itsDataSet.end()--)->first.itsAntenna2);
      TRACER1( "ant1-ant2: " << ant1 << "-" << ant2);
      TRACER1("Start of sequence: " << itsDataSet.upper_bound(FromHeader)->first.itsAntenna1 << "-" <<
              itsDataSet.upper_bound(FromHeader)->first.itsAntenna2);
      TRACER1("End of sequence: " << itsDataSet.upper_bound(ToHeader)->first.itsAntenna1 << "-" <<
              itsDataSet.upper_bound(ToHeader)->first.itsAntenna2);
      
      TRACER1("------------------------------------------------------");
      for(UVPDataSet::iterator p = itsDataSet.begin();
          p != itsDataSet.end();
          p++) {
        TRACER1(p->first.itsAntenna1 << "-" << p->first.itsAntenna2);
      }
      TRACER1("------------------------------------------------------");
#endif
      
      itsCanvas->setChannels(itsNumberOfChannels); // Number of channels. Clears buffer.
      unsigned int spectraAdded = 0;
      for(UVPDataSet::iterator p = itsDataSet.upper_bound(FromHeader);
          p != itsDataSet.upper_bound(ToHeader) && p != itsDataSet.end();
          p++) {
        const   UVPDataAtom *dataAtom = &(p->second);
#if(DEBUG_MODE)
        TRACER1( "Ant1: " <<  dataAtom->getHeader().itsAntenna1);
        TRACER1( "Ant2: " <<  dataAtom->getHeader().itsAntenna2);
        cout.precision(10);
        TRACER1( "Time: " <<  dataAtom->getHeader().itsTime); 
        cout.precision(6);
        TRACER1( "Corr: " <<  dataAtom->getHeader().itsCorrelationType);
#endif
        double *Values = new double[dataAtom->getNumberOfChannels()];
        for(unsigned int j = 0; j < dataAtom->getNumberOfChannels(); j++) {
          Values[j] = std::abs(*dataAtom->getData(j));
        }
        UVPSpectrum   Spectrum(dataAtom->getNumberOfChannels(), spectraAdded, Values);
        if(dataAtom->getHeader().itsCorrelationType == UVPDataAtomHeader::RR) {
          itsCanvas->slot_addSpectrum(Spectrum);
          spectraAdded++;
        }
        delete[] Values;
      }
      itsNumberOfTimeslots = spectraAdded;
      itsCanvas->drawView();
    }
    pass++;
    qApp->processEvents();
    
  } // while 
}
