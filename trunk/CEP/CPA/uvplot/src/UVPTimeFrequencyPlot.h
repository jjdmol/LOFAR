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

#if !defined(UVPTIMEFREQUENCYPLOT_H)
#define UVPTIMEFREQUENCYPLOT_H

// $Id$

#include <uvplot/UVPDisplayArea.h>
#include <uvplot/UVPSpectrumVector.h>
#include <uvplot/UVPDataAtomVector.h>


#include <vector>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

//! Plots a 2D image representation of spectrum-time data.
/*! Time runs from top to bottom, frequency from left to right
 */

class UVPTimeFrequencyPlot: public UVPDisplayArea
{
  Q_OBJECT
  
#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif
    
 public:
  //! Constructor
                 UVPTimeFrequencyPlot(QWidget *parent);
  

  //! Performs the actual drawing.
  virtual void drawView();

  //! Sets the number of frequency channels.
  /*! setChannels destroys any already present data. Also resets
      itsValueAxis to the default, UVPAxis().

      \param numberOfFrequencyChannels is the new number of frequency
      channels. 
   */
  void         setChannels(unsigned int numberOfChannels);

  public slots:
  
  //! Add a spectrum to itsSpectrum.
  /*! Adds spectrum to itsSpectrum. The transferfunction of
      itsValueAxis is NOT recalculated.  \param spectrum must have the
      same number of channels as defined by setChannels().
   */
  void slot_addSpectrum(const UVPSpectrum& spectrum);
  
  
  //! Add a UVPDataAtom to itsComplexSpectrum.
  /*! \param atom points to an object that MUST exist during the
      entire lifetime of itsComplexSpectrumVector. That is, at least
      until the next setChannels() call.The transferfunction of
      itsValueAxis is recalculated. It is centerred on 0.
  */  
  void slot_addDataAtom(const UVPDataAtom* atom);
 
  //! Redraws the image. Simply calls drawView().
  void slot_paletteChanged();

  signals:
 
  void signal_timeChanged(double time);
  
 protected:

  virtual void       mouseMoveEvent(QMouseEvent *event);

 private:

  UVPSpectrumVector  itsSpectrum;
  UVPDataAtomVector  itsComplexSpectrum;
  
  UVPAxis            itsValueAxis;
};

#endif // UVPTIMEFREQUENCYPLOT_H
