//#  jParmFacadeInterface.java: The RMI interface to the Parameter database.
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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
  
package nl.astron.lofar.java.cep.jparmfacade;


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.HashMap;
import java.util.Vector;

public interface jParmFacadeInterface extends Remote 
{
    // Constants
    public static final String SERVICENAME = "jParmFacade";
    
    public void setParmFacadeDB(String tableName) throws RemoteException;

    // Get the domain range (as startx,endx,starty,endy) of the given
    // parameters in the table.
    // This is the minimum start value and maximum end value for all parameters.
    // An empty name pattern is the same as * (all parm names).
    public Vector<Double> getRange(String parmNamePattern) throws RemoteException;
    
    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    public Vector<String> getNames(String parmNamePattern) throws RemoteException;
    
    // Get the parameter values for the given parameters and domain.
    // The domain is given by the start and end values, while the grid is
    // given by nx and ny.
    public HashMap<String,Vector<Double>> getValues(String parmNamePattern,
            double startx, double endx, int nx,
            double starty, double endy, int ny) throws RemoteException;
    
    // Get the parameter values for the given parameters and timeframe.
    // The domain is given by the start and end values, while the time is
    // given by startSolveTime and endSolveTime.
    public HashMap<String,Vector<Double>> getHistory(String parmNamePattern,
            double startx, double endx, double starty,
            double endy, double startSolveTime, double endSolveTime) throws RemoteException;
}
