
// Copyright notice should go here

#if !defined(UVPKARMAVIEWER_H)
#define UVPKARMAVIEWER_H

// $Id$

#include <string>

#include <X11/Intrinsic.h>      // Widget
#include <X11/StringDefs.h>     // XtNborderWidth

#include <karma.h>
#include <karma_viewimg.h>      // ViewableImage
#include <karma_xtmisc.h>
#include <karma_im.h>
#include <karma_ic.h>           // Icons
#include <Xkw/ImageDisplay.h>   // the ImageDisplay widget




Boolean UVPKarmaViewer_idleProcessing(XtPointer instance);



//! C++ wrapper for a Karma based UV data visualiser
/*!
 */
class UVPKarmaViewer
{
public:
  friend Boolean UVPKarmaViewer_idleProcessing(XtPointer instance);

   UVPKarmaViewer(int   argc,
                  char* argv[]);
  ~UVPKarmaViewer();

  void run();
  
protected:
private:

  Widget        itsMainShell;
  Widget        itsImageDisplay;
  ViewableImage itsImage;
  ViewableImage itsMagnifiedImage;

  KWorldCanvas  itsPseudoColourWorldCanvas;
  XtAppContext  itsApplicationContext;
  Display*      itsDisplay;

  std::string   itsCaption;
};



#endif // UVPKARMAVIEWER_H
