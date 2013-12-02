/*
 * IPlotDataExport.java
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
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * This interface forms the communications contract between the plotter classes
 * and any possible data export layer using a class that implements
 * this interface.
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006 11:00:00
 */
public interface IPlotDataExport{
    
    /**
     * Exports a dataset using a set of String parameters
     * @param params Object containing zero or more arguments for the export class
     * @param data the dataset to be exported
     * @throws PlotterException will be thrown if the dataset could not
     * be exported for any reason, like database exceptions, file errors, etc. 
     * (This exception will be migrated to a more specific PlotterDataExportException)
     */
    public void exportData(Object params, HashMap<String,Object> data) throws PlotterException;
    
}

