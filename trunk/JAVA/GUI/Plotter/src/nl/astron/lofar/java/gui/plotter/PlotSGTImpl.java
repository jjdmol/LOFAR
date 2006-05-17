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

import gov.noaa.pmel.sgt.DataNotFoundException;
import gov.noaa.pmel.sgt.JPane;
import gov.noaa.pmel.sgt.LineAttribute;
import gov.noaa.pmel.sgt.LineCartesianRenderer;
import gov.noaa.pmel.sgt.dm.Collection;
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
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyVetoException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
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
    
    private HashMap data;
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
        aLayout.getFirstLayer().removeAllChildren();
        aLayout.removeAll();
        aLayout = null;
        lad = null;
        pma = null;
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
    public JComponent createPlot(int type, String name, HashMap data, boolean separateLegend) throws PlotterException{
        
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
        }
        //JPanel keyAndPlotPanel = new JPanel();
        //keyAndPlotPanel.add(aNewPlot);
        //keyAndPlotPanel.add(aNewPlot,BorderLayout.CENTER);
        //JPane keyPane = aNewPlot.getKeyPane();
        
        //keyAndPlotPanel.add(keyPane,BorderLayout.SOUTH);
        //aNewPlot.setEditClasses(false);
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
    private JPlotLayout linePlot(String name, HashMap data, boolean separateLegend, boolean showPointsOnly) throws PlotterException{
        JPlotLayout layout = new JPlotLayout(JPlotLayout.LINE, false, false,
                name,null,separateLegend);
        
        try{
            
            if(separateLegend){
                layout.setKeyLayerSizeP(new Dimension2D(6.0, 1.0));
                layout.setKeyBoundsP(new Rectangle2D.Double(0.0, 1.0, 1.0, 1.0));
                layout.setKeyLocationP(new Point2D.Double(0.0, 0.0));
            }
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
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            //Loop through Metadata and pointers to XY values
            if(data != null && data.keySet().size()>0){
                Iterator it = data.keySet().iterator();
                while(it.hasNext()){
                    String key = (String)it.next();
                    if(key.equalsIgnoreCase(PlotConstants.DATASET_NAME)){
                        plotTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_SUBNAME)){
                        plotSubTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISLABEL)){
                        xAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISUNIT)){
                        xAxisUnits = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXISTYPE)){
                        xAxisType = (String)data.get(key);
                        if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                            //TODO TIME AXIS
                        } else if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_MJDTIME)){
                            //TODO TIME AXIS
                        }
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXIS_RANGE_START)){
                        xstart = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XAXIS_RANGE_END)){
                        xend = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISLABEL)){
                        yAxisTitle = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISUNIT)){
                        yAxisUnits = (String)data.get(key);
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXISTYPE)){
                        yAxisType = (String)data.get(key);
                        if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                            //TODO TIME AXIS
                        } else if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_MJDTIME)){
                            //TODO TIME AXIS
                        }
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXIS_RANGE_START)){
                        ystart = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YAXIS_RANGE_END)){
                        yend = Double.parseDouble((String)data.get(key));
                    } else if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUES)){
                        values = (HashSet<HashMap>)data.get(key);
                    }else{
                        throw new InvalidDataSetException("(Metadata was found that is not supported for a Line Plot: "+key.toString()+")");
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
                        HashMap line = (HashMap)linesIterator.next();
                        Iterator lineIterator = line.keySet().iterator();
                        
                        //Retrieve XY pair label and xy values
                        while(lineIterator.hasNext()){
                            String key = (String)lineIterator.next();
                            if(key.equalsIgnoreCase(PlotConstants.DATASET_VALUELABEL)){
                                lineLabel = (String)line.get(key);
                                
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_XVALUES)){
                                xArray = (double[])line.get(key);
                                if(xAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                                    xArrayDate = new GeoDate[xArray.length];
                                    for(int i = 0; i<xArray.length;i++){
                                        xArrayDate[i]=new GeoDate(Long.parseLong(Double.toString(xArray[i])));
                                    }
                                }
                            } else if(key.equalsIgnoreCase(PlotConstants.DATASET_YVALUES)){
                                yArray = (double[])line.get(key);
                                if(yAxisType.equals(PlotConstants.DATASET_AXIS_TYPE_TIME)){
                                    yArrayDate = new GeoDate[yArray.length];
                                    for(int i = 0; i<yArray.length;i++){
                                        yArrayDate[i]=new GeoDate(Long.parseLong(Double.toString(yArray[i])));
                                    }
                                }
                            }else{
                                throw new InvalidDataSetException("(A value array was found that is not supported for a Line Plot: "+key.toString()+")");
                            }
                        }
                        
                        SimpleLine lineData = new SimpleLine(
                                xArray,yArray,lineLabel);
                        
                        lineData.setXMetaData(meta);
                        lineData.setYMetaData(ymeta);
                        //Add line to plot
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
    private JPlotLayout gridPlot(String name, HashMap data, boolean separateLegend) throws PlotterException{
        
        
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
            
            HashSet<HashMap> values = new HashSet<HashMap>();
            
            //Loop through Metadata and pointers to XYZ values
            if(data != null && data.keySet().size()>0){
                Iterator it = data.keySet().iterator();
                while(it.hasNext()){
                    String key = (String)it.next();
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
                        values = (HashSet<HashMap>)data.get(key);
                    }else{
                        throw new InvalidDataSetException("(Metadata was found that is not supported for a Grid Plot: "+key.toString()+")");
                    }
                }
                //Set titles to plot
                
                layout.setTitles(plotTitle,plotSubTitle,"");
                
                //Loop through XYZ Value data
                if(values != null && values.size()> 0){
                    
                    Iterator linesIterator = values.iterator();
                    //Loop through all XY pairs
                    int lineNumber = 0;
                    while(linesIterator.hasNext()){
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
                        HashMap grid = (HashMap)linesIterator.next();
                        Iterator lineIterator = grid.keySet().iterator();
                        
                        //Retrieve XYZ pair label and xyz values
                        while(lineIterator.hasNext()){
                            String key = (String)lineIterator.next();
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
    private JPlotLayout scatterPlot(String name, HashMap data, boolean separateLegend) throws PlotterException{
        throw new NotImplementedException("Scatter plots are not yet implemented in the plotter's SGT plugin.");
    }
    /**
     * Returns the current dataset used in the plot
     * @return the dataset currently in use.
     */
    public HashMap getData(){
        return data;
    }
    /**
     * Sets the dataset used in the plot
     * @param newData A new set of data
     */
    public void setData(HashMap newData){
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
    public JComponent getLegend(JComponent aPlot) throws PlotterException{
        JPlotLayout parentPlot = null;
        JPane keyPane = null;
        
        try {
            
            parentPlot = (JPlotLayout) aPlot;
            keyPane = parentPlot.getKeyPane();
            Collection dataQuantity = parentPlot.getData();
            
            if(parentPlot.getId().indexOf("line_") != -1){
                //keyPane.setSize(new Dimension(parentPlot.getWidth()-50,(dataQuantity.size()*16)));
                keyPane.setSize(new Dimension(550,100+dataQuantity.size()*20));
            }else if(parentPlot.getId().indexOf("grid_") != -1){
                keyPane.setSize(new Dimension(600,100));
            }
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
                        aLayout.getKeyPane().setBatch(true);
                        aLayout.getKeyPane().setBatch(false);
                        lad.setLineAttribute(attr);
                        if(!lad.isShowing()){
                            lad.setVisible(true);
                        }
                    }
                }
            } else if(object == aLayout){
                if(e.isPopupTrigger() || e.getClickCount() == 2){
                    Object obj = aLayout.getObjectAt(e.getX(),e.getY());
                    
                    aLayout.setSelectedObject(obj);
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
                    }
                }
            }
        }
        
    }
    
}
