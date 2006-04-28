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
    
    private String message;
    /** Creates a new instance of PlotterFrameworkNotFoundException */
    public PlotterFrameworkNotFoundException(String message) {
        super();
        this.message = message;
    }
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_FRAMEWORK_NOT_FOUND + " " +message;
    }
    
    
}
