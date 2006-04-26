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
import nl.astron.lofar.sas.plotter.exceptions.PlotterDataAccessException;

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
	 * @param constraints
	 * 
	 */
	public HashMap retrieveData(String[] constraints) throws PlotterDataAccessException{
            //create the hashmap to be returned
            HashMap<String,Object> data = new HashMap<String,Object>();
            if(constraints[0].equalsIgnoreCase("line")){

                String plotTitle = "Testset LOFAR Plotter | XYLine";
                data.put(PlotConstants.DATASET_NAME,plotTitle);
                String plotSubTitle = "Using PlotDataAccessTestImpl";
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
            
            }else if (constraints[0].equalsIgnoreCase("grid")){
                
                String plotTitle = "Testset LOFAR Plotter | Grid";
                data.put(PlotConstants.DATASET_NAME,plotTitle);
                String plotSubTitle = "Using PlotDataAccessTestImpl";
                data.put(PlotConstants.DATASET_SUBNAME,plotSubTitle);
                String xAxisTitle = "t";
                data.put(PlotConstants.DATASET_XAXISLABEL,xAxisTitle);

                String xAxisUnits = "seconds";
                data.put(PlotConstants.DATASET_XAXISUNIT,xAxisUnits);

                String yAxisTitle = "Frequency";
                data.put(PlotConstants.DATASET_YAXISLABEL,yAxisTitle);

                String yAxisUnits = "MHz";
                data.put(PlotConstants.DATASET_YAXISUNIT,yAxisUnits);
                
                String zAxisTitle = "Intensity";
                data.put(PlotConstants.DATASET_ZAXISLABEL,yAxisTitle);

                String zAxisUnits = "dB";
                data.put(PlotConstants.DATASET_ZAXISUNIT,yAxisUnits);
                
                
                HashSet<HashMap> values = new HashSet<HashMap>();

                HashMap<String,Object> aGrid = new HashMap<String,Object>();
                
                 aGrid.put(PlotConstants.DATASET_VALUELABEL,"Axis of Evil");
                 double[] xArray2 = new double[100];
                 double[] yArray2 = new double[100];
                 double[] zArray2 = new double[100*100];
                 for(int i = 0; i < xArray2.length;i++){
                     xArray2[i] = i;//Math.random();
                 }
                 for(int i = 0; i < yArray2.length;i++){
                     yArray2[i] = i*i;//Math.random();
                 } 
                 for(int i = 0; i < zArray2.length;i++){
                     zArray2[i] = Math.random();
                 } 
                 /*xArray2[0] = 1;
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
                 
                 for(int i = 0; i < zArray2.length;i++){
                     zArray2[i] = Math.random();
                 }
                 */
                 aGrid.put(PlotConstants.DATASET_XVALUES,xArray2);
                 aGrid.put(PlotConstants.DATASET_YVALUES,yArray2);
                 aGrid.put(PlotConstants.DATASET_ZVALUES,zArray2);
                 values.add(aGrid);
                 data.put(PlotConstants.DATASET_VALUES,values);
            }
            return data; 
	}
}
