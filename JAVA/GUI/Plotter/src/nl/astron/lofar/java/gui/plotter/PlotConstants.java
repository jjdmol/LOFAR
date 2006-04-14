/*
 * PlotConstants.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

/**
 * @version $Id$
 * @created April 14, 2006, 2:24 PM
 * @author pompert
 */
public abstract class PlotConstants{
    
    //Data Model constants
    
    //Data Model Metadata
    public static final String DATASETNAME    = "DataSetName";
    public static final String DATASETSUBNAME = "DataSetSubName";
    public static final String DATASETXAXISLABEL = "DataSetXAxisLabel";
    public static final String DATASETXAXISUNIT  = "DataSetXAxisUnit";
    public static final String DATASETYAXISLABEL = "DataSetYAxisLabel";
    public static final String DATASETYAXISUNIT  = "DataSetYAxisUnit";
    public static final String DATASETZAXISLABEL = "DataSetZAxisLabel";
    public static final String DATASETZAXISUNIT = "DataSetZAxisUnit";
    public static final String DATASETVALUES = "DataSetValues";
    
    //Data Model ValueSets
    
    public static final String XVALUES = "XValues";
    public static final String YVALUES = "YValues";
    public static final String ZVALUES = "ZValues";
    public static final String VALUELABEL = "DataSetValueLabel";
    
    //Supported plotter framework
    public static final String FRAMEWORK = "nl.astron.lofar.sas.plotter.SGTPlot";

}
