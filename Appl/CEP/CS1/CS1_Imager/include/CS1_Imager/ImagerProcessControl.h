//#  ImagerProcessControl.h: one line description
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#  @author Adriaan Renting, renting@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFARIMAGERPROCESSCONTROL_H
#define LOFARIMAGERPROCESSCONTROL_H

#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>

/**
@author Adriaan Renting
*/
namespace casa
{
  class MeasurementSet; //foreward declaration
  class Imager;
} //namespace casa

namespace LOFAR
{
  namespace CS1
  {
    class ImagerProcessControl : public LOFAR::ACC::PLC::ProcessControl
    {
    private:
      std::string  itsMS;
      bool         itsCompress;
      std::string  itsDataMode;
      std::string  itsImageMode;
      int          itsNChannel;
      int          itsStart;
      int          itsStep;
      int          itsNX;
      int          itsNY;
      double       itsCellX;
      double       itsCellY;
      std::string  itsStokes;
      std::string  itsWeightType;
      int          itsWeightNPixels;
      int          itsTile;
      double       itsPadding;
      std::string  itsGridFunction;
      std::string  itsImageType;
      std::string  itsImageName;

      std::vector<int> itsSpectralWindows;

      casa::MeasurementSet* myMS;
      casa::Imager*         myImager;
    public:
      ImagerProcessControl(void);

      ~ImagerProcessControl(void);
      // \name Command to control the processes.
      // There are a dozen commands that can be sent to a application process
      // to control its flow. The return values for these command are:<br>
      // - True   - Command executed succesfully.
      // - False  - Command could not be executed.
      //
      // @{

      // During the \c define state the process check the contents of the
      // ParameterSet it received during start-up. When everthing seems ok the
      // process constructs the communication channels for exchanging data
      // with the other processes. The connection are NOT made in the stage.
      tribool define   (void);

      // When a process receives an \c init command it allocates the buffers it
      // needs an makes the connections with the other processes. When the
      // process succeeds in this it is ready for dataprocessing (or whatever
      // task the process has).
      tribool init     (void);

      // During the \c run phase the process does the work it is designed for.
      // The run phase stays active until another command is send.
      tribool run      (void);

      // With the \c pause command the process stops its run phase and starts
      // waiting for another command. The \c condition argument contains the
      // contition the process should use for ending the run phase. This
      // condition is a key-value pair that can eg. contain a timestamp or a
      // number of a datasample.
      tribool pause(const std::string&);

      // \c Quit stops the process.
      // The process \b must call \c unregisterAtAC at ProcControlServer during
      // the execution of this command to pass the final results to the
      // Application Controller.
      tribool quit(void);
      tribool release(void);

      // \c Recover reconstructs the process as it was saved some time earlier.
      // The \c source argument contains the database info the process must use
      // to find the information it needs.
      tribool recover(const std::string&);

      // With \c reinit the process receives a new parameterset that it must use
      // to reinitialize itself.
      tribool reinit(const  std::string&);

      // With the \c snapshot command the process is instructed to save itself
      // in a database is such a way that on another moment in time it can
      // be reconstructed and can continue it task.<br>
      // The \c destination argument contains database info the process
      // must use to save itself.
      tribool snapshot(const std::string&);

      // Define a generic way to exchange info between client and server.
      std::string askInfo(const std::string&);
      // @}
    };//class ImagerProcessControl
  } //namespace CS1
};//namespace LOFAR

#endif //LOFARIMAGERPROCESSCONTROL_H
