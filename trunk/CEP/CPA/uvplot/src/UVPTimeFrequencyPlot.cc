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
  const unsigned int Ncol(getNumberOfColors());
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
          // SOMETIMES colre > Ncol ?!?!?!
          int colre = int(itsValueAxis.worldToAxis(spectrum->real()));
          int colim = int(itsValueAxis.worldToAxis(spectrum->imag()));
          
          if(colre < Ncol && colim < Ncol && colre >= 0 && colim >= 0) {
            BufferPainter.setPen(itsComplexColormap[itsRealIndex[colre]+itsImagIndex[colim]]); 
          } else {
            std::cout << "*************************************" << std::endl;
            std::cout << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
            std::cout << "colre: " << colre << std::endl;
            std::cout << "colim: " << colim << std::endl;
            std::cout << "real : " << spectrum->real() << std::endl;
            std::cout << "imag : " << spectrum->imag() << std::endl;
            std::cout << "Ncol: " << Ncol << std::endl;
            std::cout << "*************************************" << std::endl;
          }
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
