// Copyright notice should go here

#include <uvplot/UVPTimeFrequencyPlot.h>


#include <qpainter.h>


#if(DEBUG_MODE)
InitDebugContext(UVPTimeFrequencyPlot, "DEBUG_CONTEXT");
#endif



//============>>>  UVPTimeFrequencyPlot::UVPTimeFrequencyPlot  <<<============

UVPTimeFrequencyPlot::UVPTimeFrequencyPlot(QWidget *parent)
  : UVPDisplayArea(parent),
    itsSpectrum(0),
    itsComplexSpectrum(),
    itsValueAxis()
  
{
#if(DEBUG_MODE)
  TRACERF1("");
#endif

  connect(this, SIGNAL(signal_paletteChanged()),
          this, SLOT(slot_paletteChanged()));
}




//===========>>>  UVPTimeFrequencyPlot::slot_paletteChanged  <<<===========

void UVPTimeFrequencyPlot::slot_paletteChanged()
{
#if(DEBUG_MODE)
  TRACERF1("");
#endif

  drawView();

#if(DEBUG_MODE)
  TRACERF1("End.");
#endif
}





//===========>>>  UVPTimeFrequencyPlot::slot_addSpectrum  <<<===========

void UVPTimeFrequencyPlot::slot_addSpectrum(const UVPSpectrum &spectrum)
{
  itsSpectrum.add(spectrum);

  /*  if(itsSpectrum.min() != itsSpectrum.max()) {
    itsValueAxis.calcTransferFunction(itsSpectrum.min(),
                                      itsSpectrum.max(),
                                      0,
                                      getNumberOfColors()-1);
  }
  */
}







//===========>>>  UVPTimeFrequencyPlot::slot_addDataAtom  <<<===========

void UVPTimeFrequencyPlot::slot_addDataAtom(const UVPDataAtom* atom)
{
  itsComplexSpectrum.add(atom, true); // honour flags

  if(itsComplexSpectrum.min() != itsComplexSpectrum.max()) {
    double absmin = fabs(itsComplexSpectrum.min());
    double absmax = fabs(itsComplexSpectrum.max());
    double maxabs = (absmin > absmax? absmin: absmax);

    itsValueAxis.calcTransferFunction(-maxabs,
                                      maxabs,
                                      0,
                                      getNumberOfColors()-1);
  }
}







//==================>>>  UVPTimeFrequencyPlot::drawView  <<<==================

void UVPTimeFrequencyPlot::drawView()
{
#if(DEBUG_MODE)
  TRACERF1("");
#endif
  QPainter BufferPainter;
  
  BufferPainter.begin(&itsBuffer);
  
  /*
  const unsigned int N(itsSpectrum.size());
  const unsigned int Nch(itsSpectrum.getNumberOfChannels());

#if(DEBUG_MODE)
  //  TRACER2("i = " << i);
  TRACER2("N = " << N);
  TRACER2("Nch = " << Nch);
#endif

  if(itsSpectrum.min() != itsSpectrum.max()) {
    for(unsigned int i = 0; i < N; i++) {
      
      const double*        spectrum(itsSpectrum[i].getValues());
      unsigned int         j(0);
      
      while(j < Nch ) {
        int col = int(itsValueAxis.worldToAxis(*spectrum++));
        BufferPainter.setPen(*getColor(col));
        BufferPainter.drawPoint(j, i);
        j++;
      }
    }
  }
  
  */
  const unsigned int N(itsComplexSpectrum.size());
  unsigned int Nch(0);
  QColor Blue(0,0,255);

  if(N>0) {
    Nch = itsComplexSpectrum[0]->getNumberOfChannels();
  }


  BufferPainter.eraseRect(0,0,width(), height());

  if(itsComplexSpectrum.min() != itsComplexSpectrum.max()) {
    for(unsigned int i = 0; i < N; i++) {
      
      const UVPDataAtom::ComplexType*  spectrum(itsComplexSpectrum[i]->getData(0));
      UVPDataAtom::FlagIterator        flag = itsComplexSpectrum[i]->getFlagBegin();
      unsigned int                     j(0);
      
      while(j < Nch ) {
        if(*flag) {
          BufferPainter.setPen(Blue);
        } else {
          int colre = int(itsValueAxis.worldToAxis(spectrum->real()));
          int colim = int(itsValueAxis.worldToAxis(spectrum->imag()));
          BufferPainter.setPen(itsComplexColormap[itsRealIndex[colre]+itsImagIndex[colim]]);
        }
        spectrum++;
        flag++;
        BufferPainter.drawPoint(j, i);
        j++;
      }
    } // for...
  }
  
  
  BufferPainter.end();

  bitBlt(this, 0, 0, &itsBuffer);

#if(DEBUG_MODE)
  TRACERF1("End.");
#endif
}





//================>>>  UVPTimeFrequencyPlot::setChannels  <<<================

void UVPTimeFrequencyPlot::setChannels(unsigned int numberOfChannels)
{
  itsSpectrum.clear();
  itsSpectrum        = UVPSpectrumVector(numberOfChannels);

  itsComplexSpectrum.clear();
  itsComplexSpectrum = UVPDataAtomVector();
  itsValueAxis       = UVPAxis();
}




//===================>>> UVPTimeFrequencyPlot::mouseMoveEvent  <<<===================

void UVPTimeFrequencyPlot::mouseMoveEvent(QMouseEvent *event)
{
  UVPDisplayArea::mouseMoveEvent(event);

  if(event->pos().y() >= 0 && (unsigned int)event->pos().y() < itsComplexSpectrum.size()) {
    emit signal_timeChanged(itsComplexSpectrum[event->pos().y()]->getHeader().itsTime);
  } else {
    emit signal_timeChanged(0);
  }
}
