/*
 * PlotSGTImpl.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This software uses (parts of) the SGT graphics library. 
 * SGT was developed and is maintained by Donald W. Denbo,
 * National Oceanic and Atmospheric Administration.
 * E-Mail : Donald.W.Denbo@noaa.gov
 * Website: http://www.epic.noaa.gov/java/sgt
 *
 */

package nl.astron.lofar.sas.plotter;

import gov.noaa.pmel.sgt.JPane;
import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
import gov.noaa.pmel.util.Dimension2D;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import javax.swing.JComponent;
import javax.swing.JPanel;
import nl.astron.lofar.sas.plotter.exceptions.EmptyDataSetException;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:46
 */
public class PlotSGTImpl implements IPlot{
    
        private HashMap data;
        
	public PlotSGTImpl(){
        
	}
        
	public JComponent createPlot(int type, String name, HashMap data) throws PlotterException{
	    
            JPlotLayout aNewPlot = null; 
            
            if(type==this.XYLINE){
                aNewPlot = linePlot(name,data);
            }
            else if(type==this.GRID){
                aNewPlot = gridPlot(name,data);
            }
            else if(type==this.SCATTER){
                aNewPlot = scatterPlot(name,data);
            }
            //JPanel keyAndPlotPanel = new JPanel();
            //keyAndPlotPanel.add(aNewPlot);
            //keyAndPlotPanel.add(aNewPlot,BorderLayout.CENTER);
            //JPane keyPane = aNewPlot.getKeyPane();
            
            //keyAndPlotPanel.add(keyPane,BorderLayout.SOUTH);
            
            return aNewPlot;
	}
        
        private JPlotLayout linePlot(String name, HashMap data) throws PlotterException{
            JPlotLayout layout = new JPlotLayout(JPlotLayout.LINE, false, false,
                    name,null,false);
            layout.setSize(640,480);
            //layout.setEditClasses(false);
            layout.setBatch(true);
            layout.setId(name);
            
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String yAxisTitle = "Y";                       
            String yAxisUnits = "no unit specified";
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            //Loop through Metadata and pointers to XY values
            if(data != null && data.keySet().size()>0){
                Iterator it = data.keySet().iterator();
                while(it.hasNext()){
                    String key = (String)it.next();
                    if(key.equalsIgnoreCase(PlotConstants.DATASET_NAME)){
                        plotTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_SUBNAME)){
                        plotSubTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISLABEL)){
                        xAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISUNIT)){
                        xAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISLABEL)){
                        yAxisTitle = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISUNIT)){
                        yAxisUnits = (String)data.get(key);
                    }
                    else if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUES)){
                       values = (HashSet<HashMap>)data.get(key); 
                    }
                }
                //Set titles to plot

                layout.setTitles(plotTitle,plotSubTitle,"");
               
                //Loop through X and Y Value data
                if(values != null && values.size()> 0){

                    Iterator linesIterator = values.iterator();
                    //Loop through all XY pairs
                    while(linesIterator.hasNext()){
                        
                        double[] xArray = null;
                        double[] yArray = null;
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                                   xAxisUnits,
                                                   false,
                                                   false);
                        
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                                   yAxisUnits,
                                                   false,
                                                   false);
                        String lineLabel = "Unknown value";
                        HashMap line = (HashMap)linesIterator.next();
                        Iterator lineIterator = line.keySet().iterator();
                        
                        //Retrieve XY pair label and xy values
                        while(lineIterator.hasNext()){ 
                            String key = (String)lineIterator.next();
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                            lineLabel = (String)line.get(key);
                            
                            }  
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                               xArray = (double[])line.get(key);
                            }
                            else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])line.get(key);
                            }                   
                        }
                        SimpleLine lineData = new SimpleLine(
                                xArray,yArray,lineLabel);
                        lineData.setXMetaData(meta);
                        lineData.setYMetaData(ymeta);
                      
                        //Add line to plot
                        layout.addData(lineData, lineLabel);
                    }
                }
            }else{
                //TODO LOG!
                throw new EmptyDataSetException();
                
            }
            
            layout.setBatch(false);
            return layout;
        }
        private JPlotLayout gridPlot(String name, HashMap data) throws PlotterException{
            return null;   
        }
        private JPlotLayout scatterPlot(String name, HashMap data) throws PlotterException{
            return null;
        }       
        public HashMap getData(){
            return data;
        }
        public void setData(HashMap newData){
            if(newData!=null){
                this.data = newData;
            }
        }
        
        
}