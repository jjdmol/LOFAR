// Copyright notice should go here

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
  
  
  //! Add a UVPDataATom to itsComplexSpectrum.
  /*! \param atom points to an object that MUST exist during the
      entire lifetime of itsComplexSpectrumVector. That is, at least
      until the next setChannels() call.The transferfunction of
      itsValueAxis is recalculated. It is centerred on 0.
  */  
  void slot_addDataAtom(const UVPDataAtom* atom);
 
  //! Redraws the image. Simply calls drawView().
  void slot_paletteChanged();
  
  
 protected:
 private:

  UVPSpectrumVector  itsSpectrum;
  UVPDataAtomVector  itsComplexSpectrum;
  
  UVPAxis            itsValueAxis;
};

#endif // UVPTIMEFREQUENCYPLOT_H
