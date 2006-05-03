/*
 * PlotDataManager.java
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
import java.util.Properties;
import java.util.ResourceBundle;
import nl.astron.lofar.java.gui.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessorInitializationException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessorNotCompatibleException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessorNotFoundException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:42
 */
public class PlotDataManager{
    
    private static PlotDataManager instance = null;
    private ResourceBundle plotterConfigurationFile;
    private boolean propertyFileOK;
    //private ParmDB m_ParmDB;
    
    public static PlotDataManager getInstance(){
        if(instance == null){
            instance = new PlotDataManager();
        }
        return instance;
    }
    
    private PlotDataManager(){
      propertyFileOK = false; 
      try {
            plotterConfigurationFile = ResourceBundle.getBundle(PlotConstants.RESOURCE_FILE);
            
      } catch (Exception iex) {
           //LOG!
          //iex.printStackTrace();
            
      } 
    }
    
    
    public void finalize() throws Throwable {
        instance = null;
    }
    
    /**
     * @param constraints
     *
     */
    public HashMap retrieveData(String[] constraints) throws PlotterException{
        Object aClass = null;
        IPlotDataAccess aDataAccessor = null;
        String dataAccessClass = "";
        try {
           dataAccessClass = plotterConfigurationFile.getString(("DATA_ACCESS_CLASS"));
        
        } catch (Exception iex) {
            //TODO LOG!
           PlotterDataAccessorNotFoundException exx = new PlotterDataAccessorNotFoundException("(No plotter_config.properties file found with a DATA_ACCESS_CLASS variable!)");
           exx.initCause(iex);
           throw exx; 
        }
        try {
           Class implementator;
           implementator = PlotDataManager.class.forName(dataAccessClass);
           aClass = implementator.newInstance();
           aDataAccessor = (IPlotDataAccess)aClass;
        } catch (IllegalAccessException ex) {
            //TODO Log!
            PlotterDataAccessorInitializationException exx = new PlotterDataAccessorInitializationException();
            exx.initCause(ex);
            throw exx; 
            
        } catch (ClassNotFoundException ex) {
            //TODO Log!
            PlotterDataAccessorNotFoundException exx = new PlotterDataAccessorNotFoundException("(used: "+dataAccessClass+" )");
            exx.initCause(ex);
            throw exx;           
        } catch (InstantiationException ex) {
            //TODO Log!
            PlotterDataAccessorInitializationException exx = new PlotterDataAccessorInitializationException();
            exx.initCause(ex);
            throw exx; 
        } catch (ClassCastException ex) {
            //TODO LOG!
            PlotterDataAccessorNotCompatibleException exx = new PlotterDataAccessorNotCompatibleException();
            exx.initCause(ex);
            throw exx; 
            
        }
        
        if(aDataAccessor != null){
            
            HashMap retrieveableData =
                    aDataAccessor.retrieveData(constraints);
            
            return retrieveableData;
            
        }
        return null;
    }
    
    /**
     * @param exportParams
     * @param data
     *
     */
    public void exportData(String[] exportParams, HashMap data) throws PlotterException{
        throw new NotImplementedException("Export of data is not yet implemented in this release.");
    }
    /**
     * Retrieves the bundle of strings in the plotter_config.properties file.
     * @returns ResourceBundle the plotter configuration items
     *
     */
    public ResourceBundle getPlotterConfigurationFile(){
        return plotterConfigurationFile;
    }
    
}
