#include <iostream>
using namespace std;

#include <stdlib.h>
#include <PSS3/Quality.h>
#include <PSS3/CalibratorOld.h>
#include <PSS3/Calibrator.h>


// Prototypes for different strategies

void RunStandardPSS3NewCal (int nIters);
void RunRandom3srcNewCal (int nParms, int nIters);
void RunNewCal10Sources (int nIters);
void RunStandardPSS3 (int nIters);
void RunStandardPSS3Interval (int nIters, float intv);
void RunRandom (int nParms, int nIters);
void RunSecondRipple (int nIters);
void RunSecondRipple_v2 (int nIters);
void RunSecondRipple_v3 (int nIters);
void RunStandardPSS3WithFastSaveParms (int nIters);
void RunStandardPSS3WithSaveResiduals (int nIters);
void RunPSS3DifferentSolveString (const string & solve, int nIters);
void RunPSS3SolveString1By1 (int nIters);
void RunPSS3Source1 (int nIters);
void RunPSS3Source1_v2 (int nIters);
void RunPSS3Source1_v3 (int nIters);
void RunPSS3Source1_v4 (int nIters);
void RunPSS3Source1_v5 (int nIters);
void RunPSS3Cascade (int nIters);
void RunPSS3Cascade_v2 (int nIters);
void RunPSS3KvdS (void);
void RunPSS3KvdS_v2 (void);
int RunOriginal (void);

// This is the switchboard function. Uncomment/comment as you
// like to try out different strategies.

void TryOut (void) {
  //RunStandardPSS3NewCal (10);
  //  RunRandom3srcNewCal (500, 1);
  RunNewCal10Sources (20);
  //RunRandom (20, 1);
  //RunStandardPSS3Interval (10, 360);
  //RunStandardPSS3WithSaveResiduals (20);
  //RunStandardPSS3WithFastSaveParms (20);
  //RunPSS3DifferentSolveString ("RA.CP1,DEC.CP1,StokesI.CP1,RA.CP2,DEC.CP2,StokesI.CP2,RA.CP3,DEC.CP3,StokesI.CP3,", 20);
  //RunPSS3SolveString1By1 (20);
  //RunPSS3Cascade_v2 (20);
  //RunPSS3KvdS_v2 ();
  //RunSecondRipple_v3 (20);
}



void RunStandardPSS3NewCal (int nIters) {
  try {
    cout << "********* RunStandardPSS3NewCal ************" << endl;
    LOFAR::Calibrator cal ("demo", "skymodel", "tanaka");

    cal.initialize ();

    cal.clearSolvableParms ();
    for (int i = 1; i <= 3; i ++) {
      cal.addSolvableParm ("RA", i);
      cal.addSolvableParm ("DEC", i);
      cal.addSolvableParm ("StokesI", i);
    }
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.showSettings ();

    cal.selectFirstTimeSlot ();
    while (cal.advanceTimeIntervalIterator ()) {
      cerr << "[NextInterval]:" << endl;
      for (int i = 0; i < nIters; i ++) {
        cerr << "i = " << i << ':' << endl;
        cal.run ();
        cal.showCurrentParms ();
      }
      // cal.SubtractOptimizedSources ();
      cal.commitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunRandom3srcNewCal (int nParms, int nIters) {
  try {
    cout << "********* RunRandom3srcNewCal ************" << endl;
    LOFAR::Calibrator cal ("demo", "skymodel", "tanaka");

    cal.initialize ();
    cal.showSettings ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    int choice;
    bool toggleReadPolcs = true;

    for (int h = 0; h < nParms; h ++) {
      cout << "[0] " << flush;

      choice = rand () % 9;

      cout << "[0a] " << flush;
      cout << h << " : ";
      cal.clearSolvableParms ();

      cout << "[0b] " << flush;

      switch (choice) {
      case 0: 
	cal.addSolvableParm ("RA.CP1");
	cout << "Solving RA.CP1" << endl;
	break;
      case 1: 
	cal.addSolvableParm ("RA.CP2");
	cout << "Solving RA.CP2" << endl;
	break;
      case 2: 
	cal.addSolvableParm ("RA.CP3");
	cout << "Solving RA.CP3" << endl;
	break;
      case 3: 
	cal.addSolvableParm ("DEC.CP1");
	cout << "Solving DEC.CP1" << endl;
	break;
      case 4: 
	cal.addSolvableParm ("DEC.CP2");
	cout << "Solving DEC.CP2" << endl;
	break;
      case 5: 
	cal.addSolvableParm ("DEC.CP3");
	cout << "Solving DEC.CP3" << endl;
	break;
      case 6: 
	cal.addSolvableParm ("StokesI.CP1");
	cout << "Solving StokesI.CP1" << endl;
	break;
      case 7: 
	cal.addSolvableParm ("StokesI.CP2");
	cout << "Solving StokesI.CP2" << endl;
	break;
      case 8: 
	cal.addSolvableParm ("StokesI.CP3");
	cout << "Solving StokesI.CP3" << endl;
	break;
      }
      cout << "[1] " << flush;
      cal.commitSolvableParms ();
  
      cout << "[2] " << flush;
      cal.selectFirstTimeSlot ();
      cout << "[3] " << flush;

      while (cal.advanceTimeIntervalIterator (toggleReadPolcs)) {
        toggleReadPolcs = false;
        for (int i = 0; i < nIters; i ++) {
          cout << "[4] " << flush; 
          cal.run ();
          cal.showCurrentParms ();
          cout << "[5] " << flush;
        }
        // cal.SubtractOptimizedSources ();
        cal.commitOptimizedParameters ();
        cout << "[6] " << flush;
      }
      cout << "[7] " << flush;
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunNewCal10Sources (int nIters) {
  try {
    cout << "********* RunNewCal10Sources ************" << endl;
    int i;

    LOFAR::Calibrator cal ("data/10Sources/demo1", "skymodel1", "meijeren",
			   "meqmodel1", "postgres");

    //    cal.setTimeSlot (2);
    cal.initialize ();

    cal.clearSolvableParms ();
    for (i = 1; i <= 10; i ++) {
      cal.addSolvableParm ("RA", i);
      cal.addSolvableParm ("DEC", i);
      cal.addSolvableParm ("StokesI", i);
    }
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    for (i = 1; i <= 10; i ++) {
      cal.addPeelSource (i);
    }
    cal.commitPeelSourcesAndMasks ();

    cal.showSettings ();

    cal.selectFirstTimeSlot ();
    while (cal.advanceTimeIntervalIterator ()) {
      cerr << "[NextInterval]:" << endl;
      for (i = 0; i < nIters; i ++) {
        cerr << "i = " << i << ':' << endl;
        cal.run ();
	cal.showCurrentParms ();
      }
      // cal.SubtractOptimizedSources ();
      cal.commitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunStandardPSS3 (int nIters) {
  try {
    cout << "********* RunStandardPSS3 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("{RA,DEC,StokesI}.*");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunRandom (int nParms, int nIters) {
  try {
    cout << "********* RunRandom ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    int choice;

    for (int h = 0; h < nParms; h ++) {
      cout << "[0] " << flush;

      choice = rand () % 9;

      cout << "[0a] " << flush;
      cout << h << " : ";
      cal.clearSolvableParms ();

      cout << "[0b] " << flush;

      switch (choice) {
      case 0: 
	cal.addSolvableParm ("RA.CP1");
	cout << "Solving RA.CP1" << endl;
	break;
      case 1: 
	cal.addSolvableParm ("RA.CP2");
	cout << "Solving RA.CP2" << endl;
	break;
      case 2: 
	cal.addSolvableParm ("RA.CP3");
	cout << "Solving RA.CP3" << endl;
	break;
      case 3: 
	cal.addSolvableParm ("DEC.CP1");
	cout << "Solving DEC.CP1" << endl;
	break;
      case 4: 
	cal.addSolvableParm ("DEC.CP2");
	cout << "Solving DEC.CP2" << endl;
	break;
      case 5: 
	cal.addSolvableParm ("DEC.CP3");
	cout << "Solving DEC.CP3" << endl;
	break;
      case 6: 
	cal.addSolvableParm ("StokesI.CP1");
	cout << "Solving StokesI.CP1" << endl;
	break;
      case 7: 
	cal.addSolvableParm ("StokesI.CP2");
	cout << "Solving StokesI.CP2" << endl;
	break;
      case 8: 
	cal.addSolvableParm ("StokesI.CP3");
	cout << "Solving StokesI.CP3" << endl;
	break;
      }
      cout << "[1] " << flush;
      cal.commitSolvableParms ();
  
      cout << "[2] " << flush;
      cal.resetTimeIntervalIterator ();
      cout << "[3] " << flush;

      while (cal.advanceTimeIntervalIterator (false)) {
        for (int i = 0; i < nIters; i ++) {
          cout << "[4] " << flush; 
          cal.Run ();
          cout << "[5] " << flush;
        }
        // cal.SubtractOptimizedSources ();
        cal.CommitOptimizedParameters ();
        cout << "[6] " << flush;
      }
      cout << "[7] " << flush;
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunStandardPSS3Interval (int nIters, float intv) {
  try {
    cout << "********* RunStandardPSS3Interval ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.setTimeInterval (intv);
    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("{RA,DEC,StokesI}.*");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      cout << "==> Next Interval." << endl;
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunSecondRipple (int nIters) {
  try {
    cout << "********* RunSesondRipple ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP2");
    cal.addSolvableParm ("DEC.CP2");
    cal.addSolvableParm ("StokesI.CP2");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunSecondRipple_v2 (int nIters) {
  try {
    cout << "********* RunSesondRipple_v2  ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP2");
    cal.addSolvableParm ("DEC.CP2");
    cal.addSolvableParm ("StokesI.CP2");
    cal.addSolvableParm ("RA.CP3");
    cal.addSolvableParm ("DEC.CP3");
    cal.addSolvableParm ("StokesI.CP3");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunStandardPSS3WithFastSaveParms (int nIters) {
  try {
    cout << "********* RunStandardPSS3WithFastSaveParms ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("{RA,DEC,StokesI}.*");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
        cal.CommitOptimizedParameters ();
      }
      // cal.SubtractOptimizedSources ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunStandardPSS3WithSaveResiduals (int nIters) {
  try {
    cout << "********* RunStandardPSS3WithSaveResiduals ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("{RA,DEC,StokesI}.*");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunPSS3DifferentSolveString (const string & solve, int nIters) {
  try {
    cout << "********* RunDifferentSolveString ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm (solve);
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}

void RunPSS3SolveString1By1 (int nIters) {
  try {
    cout << "********* RunPSS3SolveString1By1 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.addSolvableParm ("StokesI.CP1");
    cal.addSolvableParm ("RA.CP2");
    cal.addSolvableParm ("DEC.CP2");
    cal.addSolvableParm ("StokesI.CP2");
    cal.addSolvableParm ("RA.CP3");
    cal.addSolvableParm ("DEC.CP3");
    cal.addSolvableParm ("StokesI.CP3");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunPSS3Source1 (int nIters) {
  try {
    cout << "********* RunPSS3Source1 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunPSS3Source1_v2 (int nIters) {
  try {
    cout << "********* RunPSS3Source1_v2 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}

void RunPSS3Source1_v3 (int nIters) {
  try {
    cout << "********* RunPSS3Source1_v3 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelMask (2);
    cal.addPeelMask (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunPSS3Source1_v4 (int nIters) {
  try {
    cout << "********* RunPSS3Source1_v4 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelMask (2);
    cal.addPeelMask (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunPSS3Source1_v5 (int nIters) {
  try {
    cout << "********* RunPSS3Source1_v5 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelMask (2);
    cal.addPeelMask (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < nIters; i ++) {
        cal.Run (); cerr << i << endl;
      }
      //cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}

void RunPSS3Cascade_v2 (int nIter) {
  try {
    cout << "********* RunPSS3Cascade_v2 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    int src;
    int i;

    cal.Initialize ();

    for (i = 0; i < nIter; i ++) {
      cout << "==> Iteration: " << i << endl;
      for (src = 1; src <= 3; src ++) {
        cout << "====> Trying source " << src << endl;

	cal.clearSolvableParms ();
        cal.addSolvableParm ("RA", src);
        cal.addSolvableParm ("DEC", src);
        cal.addSolvableParm ("StokesI", src);
        cal.commitSolvableParms ();
    
        cal.clearPeelSources ();
        cal.addPeelSource (src);
        cal.commitPeelSourcesAndMasks ();
    
        cal.resetTimeIntervalIterator ();
        cal.advanceTimeIntervalIterator ();
        cal.Run ();
        cal.SubtractOptimizedSources ();
        cal.CommitOptimizedParameters ();
      }
    }
  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}


void RunPSS3KvdS (void) {
  try {
    cout << "********* RunStandardKvdS ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.ShowSettings ();

    cal.clearSolvableParms ();
    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < 10; i ++) {
        cal.Run (); cerr << i << endl;
      }
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }


    cal.clearSolvableParms ();
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      cal.Run ();
      // cal.SubtractOptimizedSources ();
      cal.CommitOptimizedParameters ();
    }

  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}



void RunPSS3KvdS_v2 (void) {
  /*
    Use with init script :
connect db='tanaka', dbtype='postgres', tablename='meqmodel'
remove *
connect db='tanaka', dbtype='postgres', tablename='skymodel'
remove *
updatedef RA.CP1 values=2.73403
updatedef DEC.CP1 values=0.453785
updatedef StokesI.CP1 values=0.8
updatedef StokesQ.CP1 values=0
updatedef StokesU.CP1 values=0
updatedef StokesV.CP1 values=0
updatedef RA.CP2 values=2.734025
updatedef DEC.CP2 values=0.453687
updatedef StokesI.CP2 values=1.0
updatedef StokesQ.CP2 values=0
updatedef StokesU.CP2 values=0
updatedef StokesV.CP2 values=0
updatedef RA.CP3 values=2.73399
updatedef DEC.CP3 values=0.4537525
updatedef StokesI.CP3 values=1.0
updatedef StokesQ.CP3 values=0
updatedef StokesU.CP3 values=0
updatedef StokesV.CP3 values=0
quit

  */
  try {
    cout << "********* RunPSS3KvdS_v2 ************" << endl;
    LOFAR::CalibratorOld cal ("demo", "meqmodel", "skymodel", "postgres", "tanaka", "");

    cal.Initialize ();

    cal.clearSolvableParms ();
    cal.addSolvableParm ("RA.CP1");
    cal.addSolvableParm ("DEC.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < 10; i ++) {
        cal.Run (); cerr << i << endl;
      }
    }

    cal.CommitOptimizedParameters ();

    cal.SubtractOptimizedSources ();
    // cal.CommitOptimizedParameters ();

    cal.clearSolvableParms ();
    cal.addSolvableParm ("StokesI.CP1");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < 10; i ++) {
        cal.Run (); cerr << i << endl;
      }
    }

    cal.CommitOptimizedParameters ();

    cal.clearSolvableParms ();
    cal.addSolvableParm ("{RA,DEC,StokesI}.*");
    cal.commitSolvableParms ();

    cal.clearPeelSources ();
    cal.addPeelSource (1);
    cal.addPeelSource (2);
    cal.addPeelSource (3);
    cal.commitPeelSourcesAndMasks ();

    cal.resetTimeIntervalIterator ();
    while (cal.advanceTimeIntervalIterator ()) {
      for (int i = 0; i < 100; i ++) {
        cal.Run (); cerr << i << endl;
      }
    }

  } catch (LOFAR::Exception & e) {
    cout << "Exception: " <<  e.what() << endl;
  } catch (std::exception & e) {
    cout << "Stnd execption: " << e.what () << endl;
  } catch (...) {
    cout << "Caught unknown exception." << endl;
  }
}



