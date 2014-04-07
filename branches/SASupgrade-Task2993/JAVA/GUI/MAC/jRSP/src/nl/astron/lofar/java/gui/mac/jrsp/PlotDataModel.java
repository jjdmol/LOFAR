/*
 * PlotDataModel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.gui.mac.jrsp;

import java.util.HashMap;
import java.util.LinkedList;
import nl.astron.lofar.java.gui.plotter.IPlotDataAccess;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterDataAccessException;

/**
 *
 * retrieveData() uses a HashMap to draw the plot. The HashMap has two key/value
 * pairs: type and data. Type is one of the type-values of this class. Data is
 * a double[] with the y-values.
 *
 * @author balken
 */
public class PlotDataModel implements IPlotDataAccess {
    
    /** Type-values. */
    public static final String SUBBAND_STATS = "subband-stats";
    public static final String BEAMLET_STATS = "beamlet-stats";
    public static final String EMPTY = "empty";
    
    /** Creates a new instance of PlotDataModel */
    public PlotDataModel() {
        HashMap<String, Object> hm = new HashMap<String, Object>();
        hm.put("type", 1);
        hm.put("data", new double[512]);
        
        
    }
    
    public void finalize() throws Throwable {
        
    }
    
    /**
     * @param constraints
     */
    public HashMap<String,Object> retrieveData(Object constraints) throws PlotterDataAccessException {
        /*
         * Change constraints to a usefull hashmap.
         */
        HashMap<String,Object> constraintsData = (HashMap<String,Object>) constraints;
        
        /*
         * Get the type and the data out of the constraints
         */
        String type = (String)constraintsData.get("type");
        double[] data = (double[])constraintsData.get("data");
        
        /*
         * Use the type to determine which retrieveData method should be used.
         */
        if (SUBBAND_STATS.equals(type)) {
            return retrieveDataSubband(data);
        } else if (BEAMLET_STATS.equals(type)) {
            return retrieveDataBeamlet(data);
        } else { // EMPTY.equals(type)
            return retrieveDataEmpty();
        }        
    }
    
    public HashMap<String,Object> retrieveDataSubband(double[] data) {
        // Create the hashmap to be returned
        HashMap<String,Object> returnData = new HashMap<String,Object>();
        
        String plotTitle = "";
        returnData.put(PlotConstants.DATASET_NAME,plotTitle);
        String plotSubTitle = "";
        returnData.put(PlotConstants.DATASET_SUBNAME,plotSubTitle);
        
        String xAxisTitle = "Frequency";
        returnData.put(PlotConstants.DATASET_XAXISLABEL,xAxisTitle);
        String xAxisUnits = "(MHz)";
        returnData.put(PlotConstants.DATASET_XAXISUNIT,xAxisUnits);
        
        String yAxisTitle = "";
        returnData.put(PlotConstants.DATASET_YAXISLABEL,yAxisTitle);
        
        String yAxisUnits = "(dB)";
        returnData.put(PlotConstants.DATASET_YAXISUNIT,yAxisUnits);
        
        LinkedList<HashMap<String,Object>> values = new LinkedList<HashMap<String,Object>>();
        
        if (data != null && data.length > 0) {
                    /*
                     * Prepare x-values.
                     */
            double[] xValues = new double[512];
            for (int j = 0; j < 512; j ++) {
                xValues[j] = j * ( 80.0 / 512);
            }
            
            for(int i = 0; i < data.length / 512; i++) {
                HashMap<String,Object> aLine = new HashMap<String,Object>();
                aLine.put(PlotConstants.DATASET_VALUELABEL,"Line " + i);
                
                double[] yValues = new double[512];
                System.arraycopy(data, i*512, yValues, 0, 512);
                
                aLine.put(PlotConstants.DATASET_XVALUES, xValues);
                aLine.put(PlotConstants.DATASET_YVALUES, yValues);
                
                values.add(aLine);
                
                returnData.put(PlotConstants.DATASET_VALUES,values);
            }
        }
        
        return returnData;
    }
    
    public HashMap<String,Object> retrieveDataBeamlet(double[] data) {
        // Create the hashmap to be returned
        HashMap<String,Object> returnData = new HashMap<String,Object>();
        
        String plotTitle = "";
        returnData.put(PlotConstants.DATASET_NAME,plotTitle);
        String plotSubTitle = "";
        returnData.put(PlotConstants.DATASET_SUBNAME,plotSubTitle);
        
        String xAxisTitle = "Beamlets";
        returnData.put(PlotConstants.DATASET_XAXISLABEL,xAxisTitle);
        String xAxisUnits = "";
        returnData.put(PlotConstants.DATASET_XAXISUNIT,xAxisUnits);
        
        String yAxisTitle = "";
        returnData.put(PlotConstants.DATASET_YAXISLABEL,yAxisTitle);
        
        String yAxisUnits = "dB";
        returnData.put(PlotConstants.DATASET_YAXISUNIT,yAxisUnits);
        
        LinkedList<HashMap<String,Object>> values = new LinkedList<HashMap<String,Object>>();
        
        if (data != null && data.length > 0) {
            /*
             * Prepare x-values.
             */
            double[] xValues = new double[ data.length ];
            for (int j = 0; j < data.length; j ++) {
                xValues[j] = j;
            }
            
            HashMap<String,Object> aLine = new HashMap<String,Object>();
            aLine.put(PlotConstants.DATASET_VALUELABEL,"Line");
                
            aLine.put(PlotConstants.DATASET_XVALUES, xValues);
            aLine.put(PlotConstants.DATASET_YVALUES, data);
                
            values.add(aLine);
                
            returnData.put(PlotConstants.DATASET_VALUES,values);            
        }
        
        return returnData;
    }
    
    public HashMap<String,Object> retrieveDataEmpty() {
        // Create the hashmap to be returned
        HashMap<String,Object> returnData = new HashMap<String,Object>();
        
        String plotTitle = "";
        returnData.put(PlotConstants.DATASET_NAME,plotTitle);
        String plotSubTitle = "";
        returnData.put(PlotConstants.DATASET_SUBNAME,plotSubTitle);
        
        String xAxisTitle = "";
        returnData.put(PlotConstants.DATASET_XAXISLABEL,xAxisTitle);
        String xAxisUnits = "";
        returnData.put(PlotConstants.DATASET_XAXISUNIT,xAxisUnits);
        
        String yAxisTitle = "";
        returnData.put(PlotConstants.DATASET_YAXISLABEL,yAxisTitle);
        
        String yAxisUnits = "";
        returnData.put(PlotConstants.DATASET_YAXISUNIT,yAxisUnits);
        
        LinkedList<HashMap> values = new LinkedList<HashMap>();
        
        return returnData;
    }
    

    public HashMap<String,Object> updateData(HashMap<String,Object> currentDataSet, Object constraints) throws PlotterDataAccessException {
        return null;
    }
}
