/*
 * PlotDataAccessParmDBImpl.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import nl.astron.lofar.sas.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006, 11:00
 */
public class PlotDataAccessParmDBImpl implements IPlotDataAccess{

       
        public PlotDataAccessParmDBImpl(){
 
	}
        

	public void finalize() throws Throwable {
          
	}

	/**
	 * @param constraint
	 * 
	 */
	public HashMap retrieveData(String constraint) throws PlotterException{
           throw new NotImplementedException("ParmDB Implementation not fully implemented"); 
	}
}