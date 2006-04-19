/*
 * PlotDataAccessTestImpl.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter.test;

import java.util.HashMap;
import java.util.HashSet;
import nl.astron.lofar.sas.plotter.IPlotDataAccess;
import nl.astron.lofar.sas.plotter.PlotConstants;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 19-04-2006, 11:00
 * @author pompert
 * @version $Id$
 * @updated 19-apr-2006, 11:00
 */
public class PlotDataAccessTestImpl implements IPlotDataAccess{

       
        public PlotDataAccessTestImpl(){
 
	}
        

	public void finalize() throws Throwable {
          
	}

	/**
	 * @param constraint
	 * 
	 */
	public HashMap retrieveData(String constraint) throws PlotterException{
            //create the hashmap to be returned
            HashMap<String,Object> data = new HashMap<String,Object>();
            
            String plotTitle = "Testset LOFAR Plotter";
            data.put(PlotConstants.DATASET_NAME,plotTitle);
            String plotSubTitle = "Using dummy data";
            data.put(PlotConstants.DATASET_SUBNAME,plotSubTitle);
            String xAxisTitle = "t";
            data.put(PlotConstants.DATASET_XAXISLABEL,xAxisTitle);
            
            String xAxisUnits = "seconds";
            data.put(PlotConstants.DATASET_XAXISUNIT,xAxisUnits);
            
            String yAxisTitle = "Frequency";
            data.put(PlotConstants.DATASET_YAXISLABEL,yAxisTitle);
            
            String yAxisUnits = "MHz";
            data.put(PlotConstants.DATASET_YAXISUNIT,yAxisUnits);
                        
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            HashMap<String,Object> aLine = new HashMap<String,Object>();
            
            aLine.put(PlotConstants.DATASET_VALUELABEL,"Lijntje 1");
             
             double[] xArray = new double[10];
             double[] yArray = new double[10];
             
             for(int i = 0;i<10;i++){
                 xArray[i] = Math.random()*10;
                 yArray[i] = Math.random()*100;
             }
             
             aLine.put(PlotConstants.DATASET_XVALUES,xArray);
             aLine.put(PlotConstants.DATASET_YVALUES,yArray);            
             values.add(aLine);
             
             HashMap<String,Object> aLine2 = new HashMap<String,Object>();
            
             aLine2.put(PlotConstants.DATASET_VALUELABEL,"Lijntje 2");
             double[] xArray2 = new double[10];
             double[] yArray2 = new double[10];
             
             xArray2[0] = 1;
             xArray2[1] = 2;
             xArray2[2] = 3;
             xArray2[3] = 4;
             xArray2[4] = 5;
             xArray2[5] = 6;
             xArray2[6] = 7;
             xArray2[7] = 8;
             xArray2[8] = 9;
             xArray2[9] = 10;
             
             yArray2[0] = xArray2[0]*xArray2[0];
             yArray2[1] = xArray2[1]*xArray2[1];
             yArray2[2] = xArray2[2]*xArray2[2];
             yArray2[3] = xArray2[3]*xArray2[3];
             yArray2[4] = xArray2[4]*xArray2[4];
             yArray2[5] = xArray2[5]*xArray2[5];
             yArray2[6] = xArray2[6]*xArray2[6];
             yArray2[7] = xArray2[7]*xArray2[7];
             yArray2[8] = xArray2[8]*xArray2[8];
             yArray2[9] = xArray2[9]*xArray2[9];
                                       
             aLine2.put(PlotConstants.DATASET_XVALUES,xArray2);
             aLine2.put(PlotConstants.DATASET_YVALUES,yArray2);            
             values.add(aLine2);    
             data.put(PlotConstants.DATASET_VALUES,values);
            return data; 
	}
}