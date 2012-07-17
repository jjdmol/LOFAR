//#  jParmFacadeAdapter.java: The RMI adapter of the OTDB database.
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

import java.util.Vector;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import java.util.HashMap;

public class jParmFacadeAdapter extends UnicastRemoteObject implements jParmFacadeInterface
{
   // Constructor
   public jParmFacadeAdapter (jParmFacade adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
    public void setParmFacadeDB(String tableName) throws RemoteException {
        adaptee.setParmFacadeDB(tableName);
    }
    
// Get the domain range (as startx,endx,starty,endy) of the given
    // parameters in the table.
    // This is the minimum start value and maximum end value for all parameters.
    // An empty name pattern is the same as * (all parm names).
    public Vector<Double> getRange(String parmNamePattern) throws RemoteException {
        Vector<Double> aV=null;
        try {
            aV=adaptee.getRange(parmNamePattern);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getRange error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aV;
    }
    
    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    public Vector<String> getNames(String parmNamePattern) throws RemoteException {
        Vector<String> aV=null;
        try {        
            aV= adaptee.getNames(parmNamePattern);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getNames error");
            anEx.initCause(ex);
            throw anEx;            
        }
        return aV;            
    }
    

    // Get the parameter values for the given parameters and domain.
    // The domain is given by the start and end values, while the grid is
    // given by nx and ny.
    public HashMap<String,Vector<Double>> getValues(String parmNamePattern,
            double startx, double endx, int nx,
            double starty, double endy, int ny) throws RemoteException {
        HashMap<String,Vector<Double>> aM=null;
        try {            
            aM=adaptee.getValues(parmNamePattern,startx,endx,nx,starty,endy,ny);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getValues error caused by: " + ex.getMessage());
            //anEx.initCause(ex);
            throw anEx;            
        }
        return aM;            
    }

    // Get the parameter values for the given parameters and timeframe.
    // The domain is given by the start and end values, while the time is
    // given by startSolvTime and endSolveTime.
    public HashMap<String,Vector<Double>> getHistory(String parmNamePattern,
            double startx, double endx, double starty, 
            double endy, double startSolveTime, double endSolveTime) throws RemoteException {
        HashMap<String,Vector<Double>> aM=null;
        try {            
            aM=adaptee.getHistory(parmNamePattern,startx,endx,starty,endy,startSolveTime,endSolveTime);
        } catch (Exception ex) {
            RemoteException anEx=new RemoteException("JNI getHistory error caused by: " + ex.getMessage());
            //anEx.initCause(ex);
            throw anEx;            
        }
        return aM;            
    }
   protected jParmFacade adaptee;   
}
