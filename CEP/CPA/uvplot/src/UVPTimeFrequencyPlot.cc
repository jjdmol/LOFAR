// Copyright notice should go here

// $ID$

#include <UVPTimeFrequencyPlot.h>


#include <qpainter.h>


#if(DEBUG_MODE)
InitDebugContext(UVPTimeFrequencyPlot, "DEBUG_CONTEXT");
#endif



//============>>>  UVPTimeFrequencyPlot::UVPTimeFrequencyPlot  <<<============

UVPTimeFrequencyPlot::UVPTimeFrequencyPlot(QWidget *parent)
  : UVPDisplayArea(parent),
    itsSpectrum(0),
    itsValueAxis()
  
{
#if(DEBUG_MODE)
  TRACER1("UVPTimeFrequencyPlot::UVPTimeFrequencyPlot");
#endif

  connect(this, SIGNAL(signal_paletteChanged()),
          this, SLOT(slot_paletteChanged()));
}




//===========>>>  UVPTimeFrequencyPlot::slot_paletteChanged  <<<===========

void UVPTimeFrequencyPlot::slot_paletteChanged()
{
#if(DEBUG_MODE)
  TRACER1("UVPTimeFrequencyPlot::slot_paletteChanged");
#endif
  drawView();
}





//===========>>>  UVPTimeFrequencyPlot::slot_addSpectrum  <<<===========

void UVPTimeFrequencyPlot::slot_addSpectrum(const UVPSpectrum &spectrum)
{
#if(DEBUG_MODE)
  TRACER1("UVPTimeFrequencyPlot::slot_addSpectrum");
  TRACER2("itsSpectrum.getNumberOfChannels(): " << itsSpectrum.getNumberOfChannels());
  TRACER2("spectrum.getNumberOfChannels(): " <<  spectrum.getNumberOfChannels()); 
#endif

  itsSpectrum.add(spectrum);
  itsValueAxis.calcTransferFunction(itsSpectrum.min(),
                                    itsSpectrum.max(),
                                    0,
                                    getNumberOfColors()-1);
}







//==================>>>  UVPTimeFrequencyPlot::drawView  <<<==================

void UVPTimeFrequencyPlot::drawView()
{
#if(DEBUG_MODE)
  TRACER1("UVPTimeFrequencyPlot::drawView");
#endif
  QPainter BufferPainter;
  
  BufferPainter.begin(&itsBuffer);
  
  const unsigned int N(itsSpectrum.size());
  const unsigned int Nch(itsSpectrum.getNumberOfChannels());

#if(DEBUG_MODE)
  //  TRACER2("i = " << i);
  TRACER2("N = " << N);
  TRACER2("Nch = " << Nch);
#endif

  for(unsigned int i = 0; i < N && itsSpectrum.min()!=itsSpectrum.max(); i++) {

    const double*        spectrum(itsSpectrum[i].getValues());
    unsigned int         j(0);

    while(j < Nch ) {
      int col = int(itsValueAxis.worldToAxis(*spectrum++));
      BufferPainter.setPen(*getColor(col));
      BufferPainter.drawPoint(j, i);
      j++;
      }
  }
  BufferPainter.end();

  bitBlt(this, 0, 0, &itsBuffer);
}





//================>>>  UVPTimeFrequencyPlot::setChannels  <<<================

void UVPTimeFrequencyPlot::setChannels(unsigned int numberOfChannels)
{
  itsSpectrum  = UVPSpectrumVector(numberOfChannels);
  itsValueAxis = UVPAxis();
}
