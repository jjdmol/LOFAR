/*
 * IPlotDataAccess.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import nl.astron.lofar.sas.plotter.exceptions.PlotterDataAccessException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006 11:00:00
 */
public interface IPlotDataAccess{
    
    /**
     * @param constraints An array of constraints to be passed to the implementating class
     * @returns HashMap The dataset that has been generated
     */
    public HashMap retrieveData(String[] constraints) throws PlotterDataAccessException;
    
}

