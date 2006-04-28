/*
 * PlotDataManager.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.util.HashMap;
import java.util.Properties;
import java.util.ResourceBundle;
import nl.astron.lofar.sas.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterDataAccessorInitializationException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterDataAccessorNotCompatibleException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterDataAccessorNotFoundException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:42
 */
public class PlotDataManager{
    
    private static PlotDataManager instance = null;
    private ResourceBundle bundle;
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
            bundle = ResourceBundle.getBundle(PlotConstants.RESOURCE_FILE);
            
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
           dataAccessClass = bundle.getString(("DATA_ACCESS_CLASS"));
        
        } catch (Exception iex) {
           //iex.printStackTrace();
           throw new PlotterDataAccessorNotFoundException("(No plotter_config.properties file found with a DATA_ACCESS_CLASS variable!)");
        }
        try {
           Class implementator;
           implementator = PlotDataManager.class.forName(dataAccessClass);
           aClass = implementator.newInstance();
           aDataAccessor = (IPlotDataAccess)aClass;
        } catch (IllegalAccessException ex) {
            //TODO Log!
            throw new PlotterDataAccessorInitializationException();
        } catch (ClassNotFoundException ex) {
            //TODO Log!
            throw new PlotterDataAccessorNotFoundException("(used: "+dataAccessClass+" )");
        } catch (InstantiationException ex) {
            //TODO Log!
            throw new PlotterDataAccessorInitializationException();
        } catch (ClassCastException ex) {
            //TODO LOG!
            throw new PlotterDataAccessorNotCompatibleException();
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
    
}
