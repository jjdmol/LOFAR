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
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import nl.astron.lofar.java.gui.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterConfigurationNotFoundException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessorInitializationException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessorNotCompatibleException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessorNotFoundException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * This singleton class functions as the gateway between the plotter and configured
 * data access and export classes. Its function is to instantiate these classes and
 * make sure that calls are redirected to them. 
 * It also loads the plotter_config.properties file and makes it available to other classes.
 * 
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:42
 */
public class PlotDataManager{
    
    private static PlotDataManager instance = null;
    private static ResourceBundle plotterConfigurationFile;
  
    private IPlotDataAccess aDataAccessor;
    private IPlotDataExport aDataExporter;
    
    /**
     * Gets the static instance of PlotDataManager. If it does not yet exist,
     * it will be created using the constructor.
     * @return the PlotDataManager instance
     */
    public static PlotDataManager getInstance(){
        if(instance == null){
            instance = new PlotDataManager();
        }
        return instance;
    }
    /**
     * Creates a new instance of PlotDataManager
     */
    private PlotDataManager(){
      
    }
    /**
     * Cleans up the data access/export interfaces and other instance variables
     */
    public void finalize() throws Throwable {
        instance = null;
        plotterConfigurationFile = null;
        aDataAccessor = null;
        aDataExporter = null;
    }
    
    /**
     * This method makes a Plotter compliant dataset using
     * the constraints provided by the PlotPanel. It will call the Data Access 
     * class provided in the plotter_config.properties file.
     * 
     * @param constraints The String array containing constraints for the data 
     * access class 
     * @return the data set generated
     * @throws PlotterException will be thrown when an Exception occurs 
     * inside the Data Access class, or when the Data Access class itself could not
     * be properly accessed due to errors in the plotter_config.properties file.
     */
    public HashMap<String,Object> retrieveData(Object constraints) throws PlotterException{
        if(aDataAccessor == null){
            initializeDataAccessLayer();
        }        
        if(aDataAccessor != null){
            
            HashMap<String,Object> retrieveableData =
                    aDataAccessor.retrieveData(constraints);
            
            return retrieveableData;
            
        }
        return null;
    }
    
    /**
     * This method updates a Plotter compliant dataset using
     * the current dataset and new constraints provided by the PlotPanel. 
     * It will call the Data Access 
     * class provided in the plotter_config.properties file.
     * 
     * @param currentData The current dataset
     * @param constraints The String array containing update constraints for the data 
     * access class 
     * @return the data set generated
     * @throws PlotterException will be thrown when an Exception occurs 
     * inside the Data Access class, or when the Data Access class itself could not
     * be properly accessed due to errors in the plotter_config.properties file.
     */
    public HashMap<String,Object> updateData(HashMap<String,Object> currentData, Object constraints) throws PlotterException{
        if(aDataAccessor == null){
            initializeDataAccessLayer();
        }        
        if(aDataAccessor != null){
            
            return aDataAccessor.updateData(currentData, constraints);
        
        }
        return null;
    }
    /**
     * This method exports a Plotter compliant dataset using
     * the parameters provided. It will call the Data Export 
     * class provided in the plotter_config.properties file.
     * 
     * @param exportParams An object containing parameters for the data 
     * export class.
     * @param data the data set to be exported
     * @throws PlotterException will be thrown when an Exception occurs 
     * inside the Data Export class, or when the Data Export class itself could not
     * be properly accessed due to errors in the plotter_config.properties file.
     */
    public void exportData(Object exportParams, HashMap<String,Object> data) throws PlotterException{
        throw new NotImplementedException("Export of data is not yet implemented in this release.");
    }
    /**
     * Retrieves the bundle of strings in the plotter_config.properties file.
     * This method is static so other classes can retrieve it , which means that the properties file can be modified
     * or extended for custom use.
     *
     * Important: The location where this method looks for is defined in the variable PlotConstants.RESOURCE_FILE
     * 
     * @see PlotConstants
     * @return The ResourceBundle of properties present in the plotter_config.properties file
     * @throws PlotterConfigurationNotFoundException will be thrown if the file could not be located or loaded.
     */
    public static ResourceBundle getPlotterConfigurationFile() throws PlotterConfigurationNotFoundException{
        if(plotterConfigurationFile == null){
            try {
                plotterConfigurationFile = ResourceBundle.getBundle(PlotConstants.RESOURCE_FILE);
            } catch (Exception iex) {
                //LOG!
                PlotterConfigurationNotFoundException exx = new PlotterConfigurationNotFoundException();
                exx.initCause(iex);
                throw exx;
                
            } 
        }
        return plotterConfigurationFile;
    }
    /**
     * Initializes the data access layer by attempting to create a new instance of the DATA_ACCESS_LAYER variable
     * in plotter_config.properties.
     * @throws PlotterException will be thrown when the Data Access class could not
     * be properly accessed due to errors in the plotter_config.properties file.
     */
    private void initializeDataAccessLayer() throws PlotterException{
        Object aClass = null;
        String dataAccessClass = "";
        ResourceBundle resources = PlotDataManager.getPlotterConfigurationFile();
        try {
            
                dataAccessClass = resources.getString(("DATA_ACCESS_CLASS"));
                Class implementator = PlotDataManager.class.forName(dataAccessClass);
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
        } catch (MissingResourceException iex) {
            //TODO LOG!
            PlotterDataAccessorNotFoundException exx = new PlotterDataAccessorNotFoundException("(The property 'DATA_ACCESS_CLASS' was not found in the plotter_config.properties file)");
            exx.initCause(iex);
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
        } catch (Exception ex) {
            //TODO Log!
            PlotterDataAccessorInitializationException exx = new PlotterDataAccessorInitializationException();
            exx.initCause(ex);
            throw exx; 
        }
    }
    /**
     * Initializes the data export layer by attempting to create a new instance of the DATA_EXPORT_LAYER variable
     * in plotter_config.properties.
     * @throws PlotterException will be thrown when the Data Export class could not
     * be properly accessed due to errors in the plotter_config.properties file.
     */
    private void initializeDataExportLayer() throws PlotterException{
        throw new NotImplementedException("Data Export Layer initialization has not been implemented yet.");
    }
    
}
