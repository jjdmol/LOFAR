/*
 *  PlotSGTImpl.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
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
 *  This software uses (parts of) the SGT graphics library.
 *  SGT was developed and is maintained by Donald W. Denbo,
 *  National Oceanic and Atmospheric Administration.
 *  E-Mail : Donald.W.Denbo@noaa.gov
 *  Website: http://www.epic.noaa.gov/java/sgt
 *
 *  NOTICE : Although this class works with the standard SGT release,
 *           it is recommended you use the supplied SGT jar file to
 *           prevent as much bugs as possible.
 */

package nl.astron.lofar.java.gui.plotter;

import gov.noaa.pmel.sgt.AxisNotFoundException;
import gov.noaa.pmel.sgt.CartesianGraph;
import gov.noaa.pmel.sgt.DataNotFoundException;
import gov.noaa.pmel.sgt.JPane;
import gov.noaa.pmel.sgt.LayerChild;
import gov.noaa.pmel.sgt.LineAttribute;
import gov.noaa.pmel.sgt.LineCartesianRenderer;
import gov.noaa.pmel.sgt.LogAxis;
import gov.noaa.pmel.sgt.LogTransform;
import gov.noaa.pmel.sgt.SGLabel;
import gov.noaa.pmel.sgt.dm.Collection;
import gov.noaa.pmel.sgt.dm.SGTData;
import gov.noaa.pmel.sgt.dm.SGTMetaData;
import gov.noaa.pmel.sgt.dm.SimpleGrid;
import gov.noaa.pmel.sgt.dm.SimpleLine;
import gov.noaa.pmel.sgt.swing.JPlotLayout;
import gov.noaa.pmel.sgt.swing.prop.LineAttributeDialog;
import gov.noaa.pmel.util.Dimension2D;
import gov.noaa.pmel.util.Rectangle2D;
import gov.noaa.pmel.util.SoTDomain;
import gov.noaa.pmel.util.SoTRange;
import gov.noaa.pmel.util.GeoDate;
import gov.noaa.pmel.util.Point2D;
import gov.noaa.pmel.util.Range2D;
import gov.noaa.pmel.util.SoTPoint;
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyVetoException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import javax.swing.JComponent;
import nl.astron.lofar.java.gui.plotter.exceptions.EmptyDataSetException;
import nl.astron.lofar.java.gui.plotter.exceptions.InvalidDataSetException;
import nl.astron.lofar.java.gui.plotter.exceptions.NotImplementedException;
import nl.astron.lofar.java.gui.plotter.exceptions.NotSupportedException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * This class provides an implementation of IPlot for use with the SGT package.
 * It manages the calls to create plots etc. in a way that SGT can support and handle.
 * <br><br>
 * This software uses (parts of) the SGT graphics library.
 * SGT was developed and is maintained by Donald W. Denbo,
 * National Oceanic and Atmospheric Administration.
 * E-Mail : Donald.W.Denbo@noaa.gov
 * Website: http://www.epic.noaa.gov/java/sgt
 * <br><br>
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:46
 */
public class PlotSGTImpl implements IPlot{
    
    private HashMap<String,Object> data;
    private JPlotLayout aLayout;
    private LineAttributeDialog lad;
    private plotMouseAdapter pma;
    
    /**
     * Creates a new instance of PlotSGTImpl
     */
    public PlotSGTImpl(){
        
    }
    /**
     * Cleans up the SGT plot(s) present in memory and other instance variables
     */
    public void finalize() throws Throwable {
        data = null;
        aLayout.clear();
        aLayout.getFirstLayer().removeAllChildren();
        aLayout.removeAll();
        aLayout = null;
        lad = null;
        pma = null;
    }
    
    /**
     * Modifies a given SGT plot using a given dataset. All data is updated, including already present data.
     * @param aPlot A plot JComponent
     * @param data The data to be displayed in the plot.
     * @return A legend JComponent of plot aPlot
     * @return the JComponent plot with the new dataset embedded.
     * @throws PlotterException will be thrown if the plot could not be generated for any reason.
     */
    @SuppressWarnings("unchecked")
    public JComponent modifyPlot(JComponent aPlot, HashMap<String,Object> data) throws PlotterException{
        
        JPlotLayout aNewPlot = (JPlotLayout)aPlot;
        
        try{
            aNewPlot.setBatch(true);
            String xAxisTitle = (String)data.get(PlotConstants.DATASET_XAXISLABEL);
            String xAxisUnits = (String)data.get(PlotConstants.DATASET_XAXISUNIT);
            String yAxisTitle = (String)data.get(PlotConstants.DATASET_YAXISLABEL);
            String yAxisUnits = (String)data.get(PlotConstants.DATASET_YAXISUNIT);
            
            LinkedList<HashMap<String,Object>> values = (LinkedList<HashMap<String,Object>>)data.get(PlotConstants.DATASET_VALUES);
            //Loop through X and Y Value data
            //System.out.println("Updating plot "+aNewPlot.getId()+": Old size :"+aNewPlot.getData().size()+" New Size: "+values.size());
            if(values != null && values.size()>= 0){
                
                if(values.size() >= aNewPlot.getData().size()){
                    //loop through all line values
                    for (HashMap<String,Object> line : values){
                        
                        double[] xArray = null;
                        double[] yArray = null;
                        GeoDate[] xArrayDate = null;
                        GeoDate[] yArrayDate = null;
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                xAxisUnits,
                                false,
                                false);
                        
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                yAxisUnits,
                                false,
                                false);
                        String lineLabel = "Unknown value";
                        
                        //Retrieve XY pair label and xy values
                        for(String key : line.keySet()){
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                                lineLabel = (String)line.get(key);
                                
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                                xArray = (double[])line.get(key);
                                
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])line.get(key);
                            }else{
                                throw new InvalidDataSetException("(A value array was found that is not supported for a Line Plot: "+key.toString()+")");
                            }
                        }
                        
                        SimpleLine lineData = new SimpleLine(
                                xArray,yArray,lineLabel);
                        lineData.setXMetaData(meta);
                        lineData.setYMetaData(ymeta);
                        lineData.setId(lineLabel);
                        
                        if(aNewPlot.getData(lineLabel) == null){
                            aNewPlot.addData(lineData, lineLabel);
                        }else{
                            SGTData someData = aNewPlot.getData(lineLabel);
                            //someData = lineData;
                            aNewPlot.clear(someData.getId());
                            aNewPlot.addData(lineData,lineLabel);
                        }
                    }
                    Collection presentData = aNewPlot.getData();
                    
                    Iterator cleanIterator = presentData.iterator();
                    while(cleanIterator.hasNext()){
                        boolean presentInUpdatedData = false;
                        SGTData someData = (SGTData)cleanIterator.next();
                        
                        Iterator cleanUpDataIterator = values.iterator();
                        //Loop through all XY pairs
                        while(cleanUpDataIterator.hasNext()){
                            HashMap line = (HashMap)cleanUpDataIterator.next();
                            String valueLabel = (String)line.get(PlotConstants.DATASET_VALUELABEL);
                            if(valueLabel.equalsIgnoreCase(someData.getId())){
                                presentInUpdatedData = true;
                            }
                        }
                        if(!presentInUpdatedData){
                            aNewPlot.clear(someData.getId());
                        }
                    }
                }else if(values.size() < aNewPlot.getData().size()){
                    //System.out.println("Removing from plot...");
                    
                    HashSet<SGTData> toBeDeletedData = new HashSet<SGTData>();
                    
                    Iterator anIterator = aNewPlot.getData().iterator();
                    while(anIterator.hasNext()){
                        SGTData someData = (SGTData)anIterator.next();
                        boolean presentInNewData = false;
                        
                        //Loop through all XY pairs
                        for(HashMap<String,Object> line : values){
                            String valueLabel = (String)line.get(PlotConstants.DATASET_VALUELABEL);
                            if(valueLabel.equalsIgnoreCase(someData.getId())){
                                presentInNewData = true;
                            }
                        }
                        if(!presentInNewData){
                            toBeDeletedData.add(someData);
                        }
                    }
                    //System.out.println("Removing "+toBeDeletedData.size()+" value from plot "+aNewPlot.getId());
                    CartesianGraph gp2 = (CartesianGraph)aNewPlot.getFirstLayer().getGraph();
                    
                    for (SGTData aData : toBeDeletedData){
                        aNewPlot.clear(aData.getId());
                    }
                }
            }
            aNewPlot.setBatch(false);
            aNewPlot.resetZoom();
            
        }catch(Exception e){
            InvalidDataSetException exx = new InvalidDataSetException("( The data set provided was not sufficient to update the line plot "+aNewPlot.getName()+". Please check the log file )");
            exx.initCause(e);
            throw exx;
        }
        
        
        return aNewPlot;
    }
    
    
    /**
     * Creates a SGT JPlotLayout plot using several key arguments
     * @param type Type of plot as dictated by PlotConstants.PLOT_*
     * @param name Name to be given to the plot
     * @param data The dataset to be used to create the plot
     * @param separateLegend Indicates the user's need for a separate legend
     * @return the JComponent plot generated
     * @throws PlotterException will be thrown if the plot could not be generated for any reason.
     */
    public JComponent createPlot(int type, String name, HashMap<String,Object> data, boolean separateLegend) throws PlotterException{
        
        JPlotLayout aNewPlot = null;
        
        if(type==PlotConstants.PLOT_XYLINE){
            aNewPlot = linePlot(name,data, separateLegend,false);
            pma = new plotMouseAdapter();
            if(aNewPlot.getKeyPane()!= null) aNewPlot.getKeyPane().addMouseListener(pma);
            aNewPlot.addMouseListener(pma);
        } else if(type==PlotConstants.PLOT_POINTS){
            aNewPlot = linePlot(name,data, separateLegend,true);
            pma = new plotMouseAdapter();
            if(aNewPlot.getKeyPane()!= null) aNewPlot.getKeyPane().addMouseListener(pma);
            aNewPlot.addMouseListener(pma);
        } else if(type==PlotConstants.PLOT_GRID){
            aNewPlot = gridPlot(name,data, separateLegend);
        } else if(type==PlotConstants.PLOT_SCATTER){
            aNewPlot = scatterPlot(name,data, separateLegend);
        } else {
            aNewPlot = linePlot(name,data, separateLegend,false);
            pma = new plotMouseAdapter();
            if(aNewPlot.getKeyPane()!= null) aNewPlot.getKeyPane().addMouseListener(pma);
            aNewPlot.addMouseListener(pma);
        }
        aLayout = aNewPlot;
        
        return aNewPlot;
    }
    /**
     * Creates a SGT Line plot using several key arguments
     * @param name Name to be given to the plot
     * @param data The dataset to be used to create the plot
     * @param separateLegend Indicates the user's need for a separate legend
     * @param showPointsOnly Indicates the user's need for a points only plot
     * @return the JPlotLayout plot generated
     * @throws PlotterException will be thrown if the plot could not be generated for any reason.
     */
    @SuppressWarnings("unchecked")
    private JPlotLayout linePlot(String name, HashMap<String,Object> data, boolean separateLegend, boolean showPointsOnly) throws PlotterException{
        JPlotLayout layout = new JPlotLayout(JPlotLayout.LINE, false, false,
                name,null,separateLegend);
        
        try{
            
            
            layout.setSize(640,480);
            
            layout.setId("line_"+name);
            layout.setName("line_"+name);
            layout.setBatch(true);
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String xAxisType = PlotConstants.DATASET_AXIS_TYPE_SPATIAL;
            String yAxisTitle = "Y";
            String yAxisUnits = "no unit specified";
            String yAxisType = PlotConstants.DATASET_AXIS_TYPE_SPATIAL;
            double xstart = -1;
            double xend = -1;
            double ystart = -1;
            double yend = -1;
            LinkedList<HashMap<String,Object>> values = new LinkedList<HashMap<String,Object>>();
            
            //Loop through Metadata and pointers to XY values
            if(data != null && data.keySet().size()>0){
                
                for(String key : data.keySet()){
                    if(key.equalsIgnoreCase(PlotConstants.DATASET_NAME)){
                        plotTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_SUBNAME)){
                        plotSubTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISLABEL)){
                        xAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXIS_RANGE_START)){
                        xstart = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXIS_RANGE_END)){
                        xend = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISUNIT)){
                        xAxisUnits = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISTYPE)){
                        xAxisType = (String)data.get(key);
                        if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                            //TODO TIME AXIS
                        } else if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_MJDTIME)){
                            //TODO TIME AXIS
                        } else if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_LOG)){
                            CartesianGraph gp = (CartesianGraph) layout.getFirstLayer().getGraph();
                            LogTransform xt = null;
                            try {
                                //System.out.println("Number of X Axes: "+ gp.getNumberXAxis());
                                xt = new LogTransform(xstart, xend, gp.getXAxis("Bottom Axis").getRangeP().start, gp.getXAxis("Bottom Axis").getRangeP().end);
                                gp.setXTransform(xt);
                                LogAxis xbot = new LogAxis(xAxisTitle);
                                xbot.setRangeU(new Range2D(xstart,xend));
                                xbot.setLocationU(new SoTPoint(gp.getXAxis("Bottom Axis").getRangeP().start, gp.getYAxis("Left Axis").getRangeP().start));
                                SGLabel xtitle = new SGLabel("xaxis title", xAxisTitle,
                                        new Point2D.Double(0.0, 0.0));
                                xtitle.setHeightP(0.2);
                                xbot.setTitle(xtitle);
                                gp.removeAllXAxes();
                                gp.addXAxis(xbot);
                                
                            } catch (AxisNotFoundException ex) {
                                ex.printStackTrace();
                            }
                            
                        }
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISLABEL)){
                        yAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISUNIT)){
                        yAxisUnits = (String)data.get(key);
                    }else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXIS_RANGE_START)){
                        ystart = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXIS_RANGE_END)){
                        yend = Double.parseDouble((String)data.get(key));
                        
                        
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISTYPE)){
                        yAxisType = (String)data.get(key);
                        if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                            //TODO TIME AXIS
                        } else if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_MJDTIME)){
                            //TODO TIME AXIS
                        } else if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_LOG)){
                            CartesianGraph gp = (CartesianGraph) layout.getFirstLayer().getGraph();
                            LogTransform yt = null;
                            try {
                                yt = new LogTransform(ystart, yend, gp.getYAxis("Left Axis").getRangeP().start, gp.getYAxis("Left Axis").getRangeP().end);
                                LogAxis xbot = new LogAxis(xAxisTitle);
                                gp.setYTransform(yt);
                                xbot.setRangeU(new Range2D(ystart,yend));
                                xbot.setLocationU(new SoTPoint(gp.getXAxis("Bottom Axis").getRangeP().start, gp.getYAxis("Left Axis").getRangeP().start));
                                SGLabel ytitle = new SGLabel("yaxis title", yAxisTitle,
                                        new Point2D.Double(0.0, 0.0));
                                ytitle.setHeightP(0.2);
                                xbot.setTitle(ytitle);
                                gp.removeAllYAxes();
                                gp.addYAxis(xbot);
                                
                                
                            } catch (AxisNotFoundException ex) {
                                ex.printStackTrace();
                            }
                            
                            
                        }
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUES)){
                        values = (LinkedList<HashMap<String,Object>>)data.get(key);
                    }else{
                        throw new InvalidDataSetException("(Metadata was found that is not supported for a Line Plot: "+key.toString()+")");
                    }
                    
                }
                //Set titles to plot
                
                layout.setTitles(plotTitle,plotSubTitle,"");
                
                //Loop through X and Y Value data
                if(values != null && values.size()> 0){
                    
                    //Loop through all XY pairs
                    for(HashMap<String,Object> line : values){
                        double[] xArray = null;
                        double[] yArray = null;
                        GeoDate[] xArrayDate = null;
                        GeoDate[] yArrayDate = null;
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                xAxisUnits,
                                false,
                                false);
                        
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                yAxisUnits,
                                false,
                                false);
                        String lineLabel = "Unknown value";
                        
                        //Retrieve XY pair label and xy values
                        for(String key : line.keySet()){
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                                lineLabel = (String)line.get(key);
                                
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                                xArray = (double[])line.get(key);
                                if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                                    xArrayDate = new GeoDate[xArray.length];
                                    for(int i = 0; i<xArray.length;i++){
                                        xArrayDate[i]=new GeoDate(Long.parseLong(Double.toString(xArray[i])));
                                    }
                                }else if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_LOG)){
                                    
                                }
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])line.get(key);
                                if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                                    yArrayDate = new GeoDate[yArray.length];
                                    for(int i = 0; i<yArray.length;i++){
                                        yArrayDate[i]=new GeoDate(Long.parseLong(Double.toString(yArray[i])));
                                    }
                                }else if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_LOG)){
                                    
                                }
                            }else{
                                throw new InvalidDataSetException("(A value array was found that is not supported for a Line Plot: "+key.toString()+")");
                            }
                        }
                        
                        SimpleLine lineData = new SimpleLine(
                                xArray,yArray,lineLabel);
                        lineData.setXMetaData(meta);
                        lineData.setYMetaData(ymeta);
                        lineData.setId(lineLabel);
                        layout.addData(lineData, lineLabel);
                        
                        if(showPointsOnly){
                            try{
                                LineAttribute marks;
                                marks = (LineAttribute) layout.getAttribute(lineData);
                                marks.setStyle(LineAttribute.MARK);
                            } catch (DataNotFoundException ex) {
                                //LOG! ex.printStackTrace();
                            }
                        }
                    }
                }
            }else{
                //TODO LOG!
                throw new EmptyDataSetException();
                
            }
            if(xstart!= -1 && xend != -1 && ystart != -1 && yend != -1){
                
                layout.setAutoRange(false,false);
                try {
                    layout.setRange(new SoTDomain(new SoTRange.Double(xstart,xend),new SoTRange.Double(ystart,yend)));
                } catch (PropertyVetoException ex) {
                    //TODO LOG!
                    ex.printStackTrace();
                }
            }
            
            layout.setName(plotTitle);
            layout.setBatch(false);
        }catch(NullPointerException e){
            //TODO LOG!
            InvalidDataSetException exx = new InvalidDataSetException("( The data set provided was not sufficient to build a Line Plot. Please check the log file )");
            exx.initCause(e);
            throw exx;
        }
        return layout;
    }
    /**
     * Creates a SGT Grid plot using several key arguments
     * @param name Name to be given to the plot
     * @param data The dataset to be used to create the plot
     * @param separateLegend Indicates the user's need for a separate legend
     * @return the JPlotLayout plot generated
     * @throws PlotterException will be thrown if the plot could not be generated for any reason.
     */
    @SuppressWarnings("unchecked")
    private JPlotLayout gridPlot(String name, HashMap<String,Object> data, boolean separateLegend) throws PlotterException{
        
        
        JPlotLayout layout = new JPlotLayout(JPlotLayout.GRID, false, false,
                name,null,separateLegend);
        try{
            
            layout.setBatch(true);
            layout.setSize(640,480);
            if(separateLegend){
                layout.setKeyLayerSizeP(new Dimension2D(6.0, 1.0));
                layout.setKeyBoundsP(new Rectangle2D.Double(0.0, 1.0, 6.0, 1.0));
            }
            layout.setId("grid_"+name);
            layout.setName("grid_"+name);
            String plotTitle = "No Title Specified";
            String plotSubTitle = "-";
            String xAxisTitle = "X";
            String xAxisUnits = "no unit specified";
            String yAxisTitle = "Y";
            String yAxisUnits = "no unit specified";
            String zAxisTitle = "Z";
            String zAxisUnits = "no unit specified";
            
            LinkedList<HashMap<String,Object>> values = new LinkedList<HashMap<String,Object>>();
            
            //Loop through Metadata and pointers to XYZ values
            if(data != null && data.keySet().size()>0){
                for(String key : data.keySet()){
                    if(key.equalsIgnoreCase(PlotConstants.DATASET_NAME)){
                        plotTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_SUBNAME)){
                        plotSubTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISLABEL)){
                        xAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISUNIT)){
                        xAxisUnits = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISLABEL)){
                        yAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISUNIT)){
                        yAxisUnits = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_ZAXISLABEL)){
                        zAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_ZAXISUNIT)){
                        zAxisUnits = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUES)){
                        values = (LinkedList<HashMap<String,Object>>)data.get(key);
                    }else{
                        throw new InvalidDataSetException("(Metadata was found that is not supported for a Grid Plot: "+key.toString()+")");
                    }
                }
                //Set titles to plot
                
                layout.setTitles(plotTitle,plotSubTitle,"");
                
                //Loop through XYZ Value data
                if(values != null && values.size()> 0){
                    int lineNumber = 0;
                    for(HashMap<String,Object> grid : values){
                        //Loop through all XY pairs
                        
                        lineNumber++;
                        double[] xArray = null;
                        double[] yArray = null;
                        double[] zArray = null;
                        
                        SGTMetaData meta = new SGTMetaData(xAxisTitle,
                                xAxisUnits,
                                false,
                                false);
                        
                        SGTMetaData ymeta = new SGTMetaData(yAxisTitle,
                                yAxisUnits,
                                false,
                                false);
                        
                        SGTMetaData zmeta = new SGTMetaData(zAxisTitle,
                                zAxisUnits,
                                false,
                                false);
                        
                        String lineLabel = "Unspecified value "+ lineNumber;
                        
                        //Retrieve XYZ pair label and xyz values
                        for(String key : grid.keySet()){
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                                lineLabel = (String)grid.get(key);
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                                xArray = (double[])grid.get(key);
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])grid.get(key);
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_ZVALUES)){
                                zArray = (double[])grid.get(key);
                            }else{
                                throw new InvalidDataSetException("(A value array was found that is not supported for a Grid Plot: "+key.toString()+")");
                            }
                        }
                        SimpleGrid gridData = new SimpleGrid(
                                zArray,xArray,yArray,lineLabel);
                        gridData.setXMetaData(meta);
                        gridData.setYMetaData(ymeta);
                        gridData.setZMetaData(zmeta);
                        gridData.setId(lineLabel);
                        //Add line to plot
                        layout.addData(gridData, lineLabel);
                    }
                }
            }else{
                //TODO LOG!
                throw new EmptyDataSetException();
                
            }
            layout.setName(plotTitle);
            layout.setBatch(false);
        }catch(NullPointerException e){
            //TODO LOG!
            InvalidDataSetException exx = new InvalidDataSetException("( The data set provided was not sufficient to build a Grid Plot. Please check the log file )");
            exx.initCause(e);
            throw exx;
        }
        return layout;
    }
    /**
     * Creates a SGT compatible Scatter plot using several key arguments
     * @param name Name to be given to the plot
     * @param data The dataset to be used to create the plot
     * @param separateLegend Indicates the user's need for a separate legend
     * @return the JPlotLayout plot generated
     * @throws PlotterException will be thrown if the plot could not be generated for any reason.
     */
    private JPlotLayout scatterPlot(String name, HashMap<String,Object> data, boolean separateLegend) throws PlotterException{
        throw new NotImplementedException("Scatter plots are not yet implemented in the plotter's SGT plugin.");
    }
    /**
     * Returns the current dataset used in the plot
     * @return the dataset currently in use.
     */
    public HashMap<String,Object> getData(){
        return data;
    }
    /**
     * Sets the dataset used in the plot
     * @param newData A new set of data
     */
    public void setData(HashMap<String,Object> newData){
        if(newData!=null){
            this.data = newData;
        }
    }
    /**
     * Create a legend/key using the SGT plot specified.
     * @param aPlot A plot JComponent (must be a SGT JPlotLayout!)
     * @return A legend JComponent of plot aPlot
     * @throws PlotterException will be thrown if the legend could not be generated for the given SGT plot.
     */
    @SuppressWarnings("unchecked")
    public JComponent getLegend(JComponent aPlot) throws PlotterException{
        JPlotLayout parentPlot = null;
        JPane keyPane = null;
        
        try {
            parentPlot = (JPlotLayout) aPlot;
            keyPane = parentPlot.getKeyPane();
            Collection dataQuantity = parentPlot.getData();
            keyPane.setBatch(true);
            if(parentPlot.getId().indexOf("line_") != -1){
                if(parentPlot.getData().size() <= 5){
                    parentPlot.setKeyLayerSizeP(new Dimension2D(6.0,1.0));
                    parentPlot.setKeyBoundsP(new Rectangle2D.Double(0.0, 1.0, 6.0, 1.0));
                    parentPlot.getKeyPane().setSize(new Dimension(600,100));
                }else if (parentPlot.getData().size() > 5){
                    parentPlot.setKeyLayerSizeP(new Dimension2D(6.0,1.0+(0.15*(parentPlot.getData().size()-5))));
                    parentPlot.setKeyBoundsP(new Rectangle2D.Double(0.0, 1.0+(0.15*(parentPlot.getData().size()-5)), 6.0, 1.0+(0.15*(parentPlot.getData().size()-5))));
                    parentPlot.getKeyPane().setSize(new Dimension(600,100+(15*(parentPlot.getData().size()-5))));
                }
            }else if(parentPlot.getId().indexOf("grid_") != -1){
                keyPane.setSize(new Dimension(600,100));
            }
            keyPane.setBatch(false);
        } catch (ClassCastException e) {
            NotSupportedException exx = new NotSupportedException("The plot ("+aPlot.getName()+") is not recognized by the plotter's configured framework (SGT). A legend can not be generated.");
            exx.initCause(e);
            throw exx;
            
        } catch (NullPointerException e) {
            NotSupportedException exx = new NotSupportedException("The plot ("+aPlot.getName()+") does not have a separate legend available.");
            exx.initCause(e);
            throw exx;
        }
        return keyPane;
    }
    /**
     * This inner class provides the SGT plot with the functionality of editing
     * line attributes at run time.
     *
     */
    class plotMouseAdapter extends MouseAdapter{
        
        /**
         * This method will listen if the user has double clicked on a line
         * in the legend of a SGT plot.
         * @param e the event being fired
         */
        @SuppressWarnings("unchecked")
        public void mouseReleased(MouseEvent e){
            Object object = e.getSource();
            if(object == aLayout.getKeyPane()){
                if(e.isPopupTrigger() || e.getClickCount() == 2){
                    Object obj = aLayout.getKeyPane().getObjectAt(e.getX(),e.getY());
                    
                    aLayout.getKeyPane().setSelectedObject(obj);
                    if(obj instanceof LineCartesianRenderer){
                        LineAttribute attr = ((LineCartesianRenderer)obj).getLineAttribute();
                        if(lad == null){
                            lad = new LineAttributeDialog();
                        }
                        SimpleLine aLine = (SimpleLine)((LineCartesianRenderer)obj).getLine();
                        lad.setTitle("Line Attribute Configuration for " + aLine.getTitle());
                        
                        lad.setLineAttribute(attr);
                        if(!lad.isShowing()){
                            lad.setVisible(true);
                            
                        }
                        aLayout.getKeyPane().setBatch(true);
                        aLayout.getKeyPane().setBatch(false);
                    }
                }
            } else if(object == aLayout){
                if(e.isPopupTrigger() || e.getClickCount() == 2){
                    Object obj = aLayout.getObjectAt(e.getX(),e.getY());
                    aLayout.setSelectedObject(obj);
                    if(obj instanceof LineCartesianRenderer){
                        SGTData data = aLayout.getData((LineCartesianRenderer)obj);
                        
                        LineAttribute attr = ((LineCartesianRenderer)obj).getLineAttribute();
                        if(lad == null){
                            lad = new LineAttributeDialog();
                        }
                        SimpleLine aLine = (SimpleLine)((LineCartesianRenderer)obj).getLine();
                        lad.setTitle("Line Attribute Configuration for " + aLine.getTitle());
                        lad.setLineAttribute(attr);
                        if(!lad.isShowing()){
                            lad.setVisible(true);
                        }
                    }
                }
            }
        }
        
    }
    
}
