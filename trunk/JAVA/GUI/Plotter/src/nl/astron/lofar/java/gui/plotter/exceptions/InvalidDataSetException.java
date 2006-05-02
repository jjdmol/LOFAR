/*
 * EmptyDataSetException.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.java.gui.plotter.exceptions;

import nl.astron.lofar.java.gui.plotter.PlotConstants;

/**
 * @version $Id$
 * @created April 18, 2006, 11:02 AM
 * @author pompert
 */
public class InvalidDataSetException extends PlotterException{
    
    private String message;
    /** Creates a new instance of InvalidDataSetException */
    public InvalidDataSetException(String message) {
        super();
        this.message = message;
    }
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_INVALID_DATASET + " "+message;
    }
    
    
}
