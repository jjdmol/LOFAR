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

package nl.astron.lofar.mac.apl.gui.jrsp;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.logging.Logger;
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
    public static final int SUBBAND_STATS = 1;
    public static final int BEAMLET_STATS = 2;
    
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
    public HashMap retrieveData(Object constraints) throws PlotterDataAccessException {
        // Create the hashmap to be returned
        HashMap<String,Object> returnData = new HashMap<String,Object>();
        
        
        /*
         * Change constraints to a usefull hashmap.
         */
        HashMap<String,Object> constraintsData = (HashMap<String,Object>) constraints;
        
   //     int type = (int)constraintsData.get("type");
        double[] data = (double[])constraintsData.get("data");
        
   //     switch (type) {
   //         case SUBBAND_STATS:
                String plotTitle = "";
                returnData.put(PlotConstants.DATASET_NAME,plotTitle);
                String plotSubTitle = "";
                returnData.put(PlotConstants.DATASET_SUBNAME,plotSubTitle);

                String xAxisTitle = "Frequency";
                returnData.put(PlotConstants.DATASET_XAXISLABEL,xAxisTitle);
                String xAxisUnits = "MHz";
                returnData.put(PlotConstants.DATASET_XAXISUNIT,xAxisUnits);

                String yAxisTitle = "?????";
                returnData.put(PlotConstants.DATASET_YAXISLABEL,yAxisTitle);

                String yAxisUnits = "Db";
                returnData.put(PlotConstants.DATASET_YAXISUNIT,yAxisUnits);

                LinkedList<HashMap> values = new LinkedList<HashMap>();

                if (data != null && data.length > 0) {
                    /*
                     * Prepare x-values.
                     */
                    double[] xValues = new double[512];
                    for (int j = 0; j < 512; j ++)
                    {
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
                          
   //     }
        
        
        return returnData;  
    }
}
