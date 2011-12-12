//# DPRun.cc: Class to run steps like averaging and flagging on an MS
//# Copyright (C) 2010
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <DPPP/DPRun.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <DPPP/MSReader.h>
#include <DPPP/MultiMSReader.h>
#include <DPPP/MSWriter.h>
#include <DPPP/MSUpdater.h>
#include <DPPP/Averager.h>
#include <DPPP/MedFlagger.h>
#include <DPPP/AORFlagger.h>
#include <DPPP/PreFlagger.h>
#include <DPPP/UVWFlagger.h>
#include <DPPP/PhaseShift.h>
#include <DPPP/Demixer.h>
///#include <DPPP/Estimator.h>
#include <DPPP/Counter.h>
#include <DPPP/ParSet.h>
#include <DPPP/ProgressMeter.h>
#include <DPPP/DPLogger.h>
#include <Common/Timer.h>
#include <Common/StreamUtil.h>

#include <casa/OS/Path.h>
#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/Timer.h>

namespace LOFAR {
  namespace DPPP {

    void DPRun::execute (const string& parsetName)
    {
      casa::Timer timer;
      NSTimer nstimer;
      nstimer.start();
      ParSet parset ((ParameterSet(parsetName)));
      DPLogger::useLogger = parset.getBool ("uselogger", false);
      bool checkparset    = parset.getBool ("checkparset", false);
      bool showProgress   = parset.getBool ("showprogress", true);
      bool showTimings    = parset.getBool ("showtimings", true);
      string msName;
      DPStep::ShPtr firstStep = makeSteps (parset, msName);
      // Show the steps and determine DPInfo again to get #times
      // to process.
      DPInfo info;
      DPStep::ShPtr step = firstStep;
      while (step) {
        step->updateInfo (info);
        ostringstream os;
        step->show (os);
        DPLOG_INFO (os.str(), true);
        step = step->getNextStep();
      }
      // Show unused parameters (might be misspelled).
      vector<string> unused = parset.unusedKeys();
      if (! unused.empty()) {
        DPLOG_WARN_STR
          (endl
           << "*** WARNING: the following parset keywords were not used ***"
           << endl
           << "             maybe they are misspelled"
           << endl
           << "    " << unused << endl);
        ASSERTSTR (!checkparset, "Unused parset keywords found");
      }
      // Process until the end.
      uint ntodo = info.ntime() * info.ntimeAvg();
      DPLOG_INFO_STR ("Processing " << ntodo << " time slots ...");
      {
        ProgressMeter* progress = 0;
        if (showProgress) {
          progress = new ProgressMeter(0.0, ntodo, "NDPPP",
                                       "Time slots processed",
                                       "", "", true, 1);
        }
        double ndone = 0;
        if (showProgress  &&  ntodo > 0) {
          progress->update (ndone, true);
        }
        DPBuffer buf;
        while (firstStep->process (buf)) {
          ++ndone;
          if (showProgress  &&  ntodo > 0) {
            progress->update (ndone, true);
          }
        }
        delete progress;
      }
      // Finish the processing.
      DPLOG_INFO_STR ("Finishing processing ...");
      firstStep->finish();
      // Give all steps the option to add something to the MS written.
      // Currently it is used by the AOFlagger to write its statistics.
      if (! msName.empty()) {
        step = firstStep;
        while (step) {
          step->addToMS (msName);
          step = step->getNextStep();
        }
      }
      // Show the counts where needed.
      step = firstStep;
      while (step) {
        ostringstream os;
        step->showCounts (os);
        DPLOG_INFO (os.str(), true);
        step = step->getNextStep();
      }
      // Show the overall timer.
      nstimer.stop();
      double duration = nstimer.getElapsed();
      ostringstream ostr;
      ostr << endl;
      timer.show (ostr, "Total NDPPP time");
      DPLOG_INFO (ostr.str(), true);
      if (showTimings) {
        // Show the timings per step.
        step = firstStep;
        while (step) {
          ostringstream os;
          step->showTimings (os, duration);
          DPLOG_INFO (os.str(), true);
          step = step->getNextStep();
        }
      }
      // The destructors are called automatically at this point.
    }

    DPStep::ShPtr DPRun::makeSteps (const ParSet& parset, string& msName)
    {
      DPStep::ShPtr firstStep;
      DPStep::ShPtr lastStep;
      // Get input and output MS name.
      // Those parameters were always called msin and msout.
      // However, SAS/MAC cannot handle a parameter and a group with the same
      // name, hence one can also use msin.name and msout.name.
      vector<string> inNames = parset.getStringVector ("msin.name",
                                                       vector<string>());
      if (inNames.empty()) {
        inNames = parset.getStringVector ("msin");
      }
      ASSERTSTR (inNames.size() > 0, "No input MeasurementSets given");
      // Find all file names matching a possibly wildcarded input name.
      // This is only possible if a single name is given.
      if (inNames.size() == 1) {
        if (inNames[0].find_first_of ("*?{['") != string::npos) {
          vector<string> names;
          names.reserve (80);
          casa::Path path(inNames[0]);
          casa::String dirName(path.dirName());
          casa::Directory dir(dirName);
          // Use the basename as the file name pattern.
          casa::DirectoryIterator dirIter (dir,
                                           casa::Regex::fromPattern(path.baseName()));
          while (!dirIter.pastEnd()) {
            names.push_back (dirName + '/' + dirIter.name());
            dirIter++;
          }
          ASSERTSTR (!names.empty(), "No datasets found matching msin "
                     << inNames[0]);
          inNames = names;
        }
      }
      string outName = parset.getString ("msout.name", "");
      if (outName.empty()) {
        outName = parset.getString ("msout");
      }
      // A write should always be done if an output name is given.
      // A name equal to . or input name means an update, so clear outname.
      bool needWrite = false;
      if (! outName.empty()) {
	needWrite = true;
	if (outName == ".") {
	  outName = "";
	} else {
	  casa::Path pathIn (inNames[0]);
	  casa::Path pathOut(outName);
	  if (pathIn.absoluteName() == pathOut.absoluteName()) {
	    outName = "";
	  }
	}
      }
      // Get the steps.
      vector<string> steps = parset.getStringVector ("steps");
      // Currently the input MS must be given.
      // In the future it might be possible to have a simulation step instead.
      // Create MSReader step if input ms given.
      MSReader* reader = 0;
      if (inNames.size() == 1) {
        reader = new MSReader (inNames[0], parset, "msin.");
      } else {
        reader = new MultiMSReader (inNames, parset, "msin.");
      }
      firstStep = DPStep::ShPtr (reader);
      lastStep = firstStep;
      // Create the other steps.
      for (vector<string>::const_iterator iter = steps.begin();
           iter != steps.end(); ++iter) {
        string prefix(*iter + '.');
        // The name is the default step type.
        string type = toLower(parset.getString (prefix+"type", *iter));
        DPStep::ShPtr step;
        if (type == "averager"  ||  type == "average"  ||  type == "squash") {
          step = DPStep::ShPtr(new Averager (reader, parset, prefix));
        } else if (type == "madflagger"  ||  type == "madflag") {
          step = DPStep::ShPtr(new MedFlagger (reader, parset, prefix));
        } else if (type == "aoflagger"  ||  type == "aoflag"
                   ||  type == "rficonsole") {
          step = DPStep::ShPtr(new AORFlagger (reader, parset, prefix));
        } else if (type == "preflagger"  ||  type == "preflag") {
          step = DPStep::ShPtr(new PreFlagger (reader, parset, prefix));
        } else if (type == "uvwflagger"  ||  type == "uvwflag") {
          step = DPStep::ShPtr(new UVWFlagger (reader, parset, prefix));
        } else if (type == "counter"  ||  type == "count") {
          step = DPStep::ShPtr(new Counter (reader, parset, prefix));
        } else if (type == "phaseshifter"  ||  type == "phaseshift") {
          step = DPStep::ShPtr(new PhaseShift (reader, parset, prefix));
        } else if (type == "demixer"  ||  type == "demix") {
          step = DPStep::ShPtr(new Demixer (reader, parset, prefix));
          ///        } else if (type == "estimate"  ||  type == "estimator") {
          ///          step = DPStep::ShPtr(new Estimator (reader, parset, prefix));
        } else {
          THROW (LOFAR::Exception, "DPPP step type " << type << " is unknown");
        }
        lastStep->setNextStep (step);
        lastStep = step;
        // Define as first step if not defined yet.
        if (!firstStep) {
          firstStep = step;
        }
      }
      // Let all steps update their info.
      DPInfo info;
      DPStep::ShPtr step = firstStep;
      while (step) {
        step->updateInfo (info);
        step = step->getNextStep();
      }
      // Tell the reader if visibility data needs to be read.
      reader->setReadVisData (info.needVisData());
      // Create an updater step if an input MS was given; otherwise a writer.
      // Create an updater step only if needed (e.g. not if only count is done).
      // If the user specified an output name, a writer is always created 
      // If there is a writer, the reader needs to read the visibility data.
      if (outName.empty()) {
        ASSERTSTR (info.nchanAvg() == 1  &&  info.ntimeAvg() == 1,
                   "A new MS has to be given in msout if averaging is done");
        ASSERTSTR (info.phaseCenterIsOriginal(),
                   "A new MS has to be given in msout if a phase shift is done");
        if (needWrite  ||  info.needWrite()) {
          ASSERTSTR (inNames.size() == 1,
                     "No update can be done if multiple input MSs are used");
          step = DPStep::ShPtr(new MSUpdater (reader, parset, "msout."));
          msName = inNames[0];
        } else {
          step = DPStep::ShPtr(new NullStep());
        }
      } else {
        step = DPStep::ShPtr(new MSWriter (reader, outName, info,
                                           parset, "msout."));
        reader->setReadVisData (true);
        msName = outName;
      }
      lastStep->setNextStep (step);
      lastStep = step;
      // Add a null step, so the last step can use getNextStep->process().
      DPStep::ShPtr nullStep(new NullStep());
      if (lastStep) {
        lastStep->setNextStep (nullStep);
      } else {
        firstStep = nullStep;
      }
      return firstStep;
    }

  } //# end namespace
}
