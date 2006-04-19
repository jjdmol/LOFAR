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
    public static final String DATASET_NAME    = "DataSetName";
    public static final String DATASET_SUBNAME = "DataSetSubName";
    public static final String DATASET_XAXISLABEL = "DataSetXAxisLabel";
    public static final String DATASET_XAXISUNIT  = "DataSetXAxisUnit";
    public static final String DATASET_YAXISLABEL = "DataSetYAxisLabel";
    public static final String DATASET_YAXISUNIT  = "DataSetYAxisUnit";
    public static final String DATASET_ZAXISLABEL = "DataSetZAxisLabel";
    public static final String DATASET_ZAXISUNIT = "DataSetZAxisUnit";
    public static final String DATASET_VALUES = "DataSetValues";
    
    //Data Model ValueSets
    
    public static final String DATASET_XVALUES = "XValues";
    public static final String DATASET_YVALUES = "YValues";
    public static final String DATASET_ZVALUES = "ZValues";
    public static final String DATASET_VALUELABEL = "DataSetValueLabel";
    
    //Supported plotter framework
    public static final String FRAMEWORK = "nl.astron.lofar.sas.plotter.SGTPlot";

    //Exception messages
    public static final String EXCEPTION_GENERIC = "An internal error occurred with no leads to its cause. Please contact software support.";
    public static final String EXCEPTION_EMPTYDATASET = "An internal error occurred : An empty data set was passed to the plotter framework.";
    public static final String EXCEPTION_FRAMEWORKINIT = "An internal error occurred : The plotter framework class specified in the constants file could not be accessed and/or instantiated by the plotter.";
    public static final String EXCEPTION_FRAMEWORKNOTFOUND = "An internal error occurred : The plotter framework class specified in the constants file could not be found by the plotter.";
    public static final String EXCEPTION_FRAMEWORKNOTCOMPATIBLE = "An internal error occurred : The plotter framework class specified in the constants file does not implement the IPlot interface, and therefore is NOT compatible with the plotter.";
    
}
