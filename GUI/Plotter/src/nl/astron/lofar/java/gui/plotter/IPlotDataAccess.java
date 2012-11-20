/*
 * IPlotDataAccess.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl

 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package nl.astron.lofar.java.gui.plotter;

import java.util.HashMap;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessException;

/**
 * This interface forms the communications contract between the plotter classes
 * and any possible data access layer using a class that implements
 * this interface.
 *
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006 11:00:00
 */
public interface IPlotDataAccess{
    
    /**
     * This method will retrieve data from the data access layer using an object with any
     * possible information and it must return a HashMap dataset understandable by the plotter. 
     * @param constraints An object with constraints to be passed to the implementing class
     * @return The dataset that has been generated
     * @throws PlotterDataAccessException will be thrown if the dataset could not
     * be generated for any reason, like database exceptions, file errors, etc.
     */
    public HashMap retrieveData(Object constraints) throws PlotterDataAccessException;
    
    /**
     * This method will update the dataset provided using an object with any
     * possible information and it must return a HashMap dataset understandable by the plotter. 
     * @param currentDataSet The dataset to be updated
     * @param constraints An object with constraints to be passed to the implementing class
     * @return The dataset that has been generated
     * @throws PlotterDataAccessException will be thrown if the dataset could not
     * be generated for any reason, like database exceptions, file errors, etc.
     */
    public HashMap updateData(HashMap<String,Object> currentDataSet, Object constraints) throws PlotterDataAccessException;
    
}

