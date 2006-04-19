/*
 * PlotterFrameworkNotFoundException.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter.exceptions;

import nl.astron.lofar.sas.plotter.PlotConstants;

/**
 * @version $Id$
 * @created April 18, 2006, 11:02 AM
 * @author pompert
 */
public class PlotterFrameworkNotFoundException extends PlotterException{
    
    /** Creates a new instance of PlotterFrameworkNotFoundException */
    public PlotterFrameworkNotFoundException() {
        super();
    }
    public String getMessage(){
        return PlotConstants.EXCEPTION_FRAMEWORKNOTFOUND;
    }
    
    
}
