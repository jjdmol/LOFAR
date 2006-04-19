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
    //private ParmDB m_ParmDB;
    
    public static PlotDataManager getInstance(){
        if(instance == null){
            instance = new PlotDataManager();
        }
        return instance;
    }
    
    private PlotDataManager(){
        
    }
    
    
    public void finalize() throws Throwable {
        instance = null;
    }
    
    /**
     * @param constraint
     *
     */
    public HashMap retrieveData(String constraint) throws PlotterException{
        Object aClass = null;
        IPlotDataAccess aDataAccessor = null;
        try {
            Class implementator = PlotDataManager.class.forName(PlotConstants.DATA_ACCESS_CLASS);
            aClass = implementator.newInstance();
            aDataAccessor = (IPlotDataAccess)aClass;
        } catch (IllegalAccessException ex) {
            //TODO Log!
            throw new PlotterDataAccessorInitializationException();
        } catch (ClassNotFoundException ex) {
            //TODO Log!
            throw new PlotterDataAccessorNotFoundException();
        } catch (InstantiationException ex) {
            //TODO Log!
            throw new PlotterDataAccessorInitializationException();
        } catch (ClassCastException ex) {
            //TODO LOG!
            throw new PlotterDataAccessorNotCompatibleException();
        }
        
        if(aDataAccessor != null){
            
            HashMap retrieveableData =
                    aDataAccessor.retrieveData(constraint);
            
            return retrieveableData;
            
        }
        return null;
    }
    
    /**
     * @param rootParams
     * @param data
     *
     */
    public void exportData(String[] rootParams, HashMap data) throws PlotterException{
        throw new NotImplementedException("Export of data is not yet implemented in this release.");
    }
    
}