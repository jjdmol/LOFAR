// Copyright notice should go here

#if !defined(UVPTIMEFREQUENCYPLOT_H)
#define UVPTIMEFREQUENCYPLOT_H

// $Id$

#include <UVPDisplayArea.h>
#include <UVPSpectrumVector.h>


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
       itsValueAxis is recalculated.
       \param spectrum must have the same number of channels as defined
       by setChannels().
   */
  void slot_addSpectrum(const UVPSpectrum &spectrum);
 
 //! Redraws the image. Simply calls drawView().
  void slot_paletteChanged();
  
  
 protected:
 private:

  UVPSpectrumVector  itsSpectrum;
  
  UVPAxis            itsValueAxis;
};

#endif // UVPTIMEFREQUENCYPLOT_H
