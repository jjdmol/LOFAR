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
    public static final String DATASET_NAME              = "DataSetName";
    public static final String DATASET_SUBNAME           = "DataSetSubName";
    
    public static final String DATASET_XAXISLABEL        = "DataSetXAxisLabel";
    public static final String DATASET_XAXISUNIT         = "DataSetXAxisUnit";
    public static final String DATASET_XAXISTYPE         = "DataSetXAxisType";
    public static final String DATASET_XAXIS_RANGE_START = "DataSetXAxisRangeStart";
    public static final String DATASET_XAXIS_RANGE_END   = "DataSetXAxisRangeEnd";
    
    public static final String DATASET_YAXISLABEL        = "DataSetYAxisLabel";
    public static final String DATASET_YAXISUNIT         = "DataSetYAxisUnit";
    public static final String DATASET_YAXISTYPE         = "DataSetYAxisType";
    public static final String DATASET_YAXIS_RANGE_START = "DataSetYAxisRangeStart";
    public static final String DATASET_YAXIS_RANGE_END   = "DataSetYAxisRangeEnd";
    
    public static final String DATASET_ZAXISLABEL        = "DataSetZAxisLabel";
    public static final String DATASET_ZAXISUNIT         = "DataSetZAxisUnit";
    public static final String DATASET_ZAXISTYPE         = "DataSetZAxisType";
    public static final String DATASET_ZAXIS_RANGE_START = "DataSetZAxisRangeStart";
    public static final String DATASET_ZAXIS_RANGE_END   = "DataSetZAxisRangeEnd";
    
    public static final String DATASET_VALUES     = "DataSetValues";
    
    //Data Model ValueSets
    
    public static final String DATASET_XVALUES    = "XValues";
    public static final String DATASET_YVALUES    = "YValues";
    public static final String DATASET_ZVALUES    = "ZValues";
    public static final String DATASET_VALUELABEL = "DataSetValueLabel";
    
    //Identifiers for plot types
    public static final int PLOT_BAR = 1;
    public static final int PLOT_XYLINE = 2;
    public static final int PLOT_SCATTER = 3;
    public static final int PLOT_GRID = 4;
    public static final boolean PLOT_SEPARATE_LEGEND = true;
    
    //Supported plotter framework
    public static final String FRAMEWORK         = "nl.astron.lofar.sas.plotter.PlotSGTImpl";
    
    //Supported plotter data access class
    public static final String DATA_ACCESS_CLASS = "nl.astron.lofar.sas.plotter.PlotDataAccessParmDBImpl";
    //public static final String DATA_ACCESS_CLASS = "nl.astron.lofar.sas.plotter.test.PlotDataAccessTestImpl";
    
    //Location of ParmDB table file(s)
    public static final String DATA_INMEP_FILE_PATH = "/home/pompert/transfer/tParmFacade.in_mep";
        
    //Supported plotter data export class
    //public static final String DATA_EXPORT_CLASS = "nl.astron.lofar.sas.plotter.PlotDataExportRootImpl";

    //Exception messages
    public static final String EXCEPTION_GENERIC                = "An internal error occurred. ";
    public static final String EXCEPTION_NOT_IMPLEMENTED         = "The plotter has detected that the feature you requested is not (fully) implemented in this release. ";
    public static final String EXCEPTION_OPERATION_NOT_SUPPORTED  = "The plotter has detected that the operation you requested is not supported. ";
    
    public static final String EXCEPTION_EMPTY_DATASET           = "An empty data set was passed to the plotter framework. ";
    
    public static final String EXCEPTION_FRAMEWORK_INIT          = "The plotter framework class specified in the constants file could not be accessed and/or instantiated by the plotter. ";
    public static final String EXCEPTION_FRAMEWORK_NOT_FOUND      = "The plotter framework class specified in the constants file could not be found by the plotter. ";
    public static final String EXCEPTION_FRAMEWORK_NOT_COMPATIBLE = "The plotter framework class specified in the constants file does not implement the IPlot interface, and therefore is NOT compatible with the plotter. ";
    
    public static final String EXCEPTION_DATA_ACCESS_INIT           = "The plotter data access class specified in the constants file could not be accessed and/or instantiated by the plotter. ";
    public static final String EXCEPTION_DATA_ACCESS_NOT_FOUND      = "The plotter data access class specified in the constants file could not be found by the plotter. ";
    public static final String EXCEPTION_DATA_ACCESS_NOT_COMPATIBLE = "The plotter data access class specified in the constants file does not implement the IPlotDataAccess interface, and therefore is NOT compatible with the plotter. ";
}
