
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
