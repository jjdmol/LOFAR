
// Copyright notice should go here

#include <UVPKarmaViewer.h>

#include <k_version.h>

#if(DEBUG_MODE)
#include <cassert>
#endif

#include <iostream>


String fallback_resources[] = {
  "UVPKarmaViewer*pseudoColourCanvas*background:              black",
  0
};



//=================>>>  UVPKarmaViewer::UVPKarmaViewer  <<<=================

UVPKarmaViewer::UVPKarmaViewer(int   argc,
                               char* argv[])
  : itsMainShell(0),
    itsImageDisplay(0),
    itsImage(0),
    itsMagnifiedImage(0),
    itsDisplay(0)
{
  std::cout << "Registering Module" << std::endl;

  im_register_module_name("UVPKarmaViewer");
  im_register_module_version_date("$Id$");
  im_register_lib_version(KARMA_VERSION);

  // Start up Xt:

  std::cout << "Create main shell" << std::endl;

  itsMainShell = xtmisc_init_app_initialise(&itsApplicationContext,
                                            "UVPKarmaViewer",
                                            0, // Array of options
                                            0, // Number of options
                                            &argc, argv,
                                            fallback_resources, // Predefined resources, colours
                                            XTMISC_INIT_ATT_MIN_CCELLS, 100,
                                            XTMISC_INIT_ATT_COMMS_SETUP, TRUE,
                                            XTMISC_INIT_ATT_END,
                                            0);
  std::cout << "Set icon" << std::endl;

  xtmisc_set_icon(itsMainShell, ic_write_kimage_icon);
  
  itsDisplay = XtDisplay(itsMainShell);

  itsCaption = "$Id$";

  /* Set the mainwindow caption */
  std::cout << "Set caption" << std::endl;
  XtVaSetValues(itsMainShell, XtNtitle, itsCaption.c_str(), 0);

  std::cout << "Create image display" << std::endl;
  itsImageDisplay = XtVaCreateManagedWidget("topWidget", // Just a name
                                            imageDisplayWidgetClass,
                                            itsMainShell
//                                            XtNborderWidth, 0,
//                                            XkwNenableAnimation, FALSE,
//                                            XkwNnumTrackLabels, 3,
                                            );
}


//=================>>>  UVPKarmaviewer::~UVPKarmaViewer  <<<=================

UVPKarmaViewer::~UVPKarmaViewer()
{
}





//=================>>>  UVPKarmaViewer::run  <<<=================

void UVPKarmaViewer::run()
{
  XEvent *theEvent;
  std::cout << "Add work function" << std::endl;
  XtWorkProcId ID = XtAppAddWorkProc(itsApplicationContext,
                                     UVPKarmaViewer_idleProcessing,
                                     this);

  std::cout << "Start main loop" << std::endl;
  while(!XtAppGetExitFlag(itsApplicationContext)) {
    XtAppNextEvent(itsApplicationContext, theEvent);
    XtDispatchEvent(theEvent);
  }

  XtRemoveWorkProc(ID);
}






//=================>>>  UVPKarmaViewer_idleProcessing  <<<=================

Boolean UVPKarmaViewer_idleProcessing(XtPointer instance)
{
  UVPKarmaViewer *This = (UVPKarmaViewer*)instance;
#if(DEBUG_MODE)
  assert(This != 0);
#endif
  // Do something useful and fast

  // We're not yet done, please call us again...
  return false;
}
