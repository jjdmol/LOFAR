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
    public static final String DATASET_NAME       = "DataSetName";
    public static final String DATASET_SUBNAME    = "DataSetSubName";
    public static final String DATASET_XAXISLABEL = "DataSetXAxisLabel";
    public static final String DATASET_XAXISUNIT  = "DataSetXAxisUnit";
    public static final String DATASET_YAXISLABEL = "DataSetYAxisLabel";
    public static final String DATASET_YAXISUNIT  = "DataSetYAxisUnit";
    public static final String DATASET_ZAXISLABEL = "DataSetZAxisLabel";
    public static final String DATASET_ZAXISUNIT  = "DataSetZAxisUnit";
    public static final String DATASET_VALUES     = "DataSetValues";
    
    //Data Model ValueSets
    
    public static final String DATASET_XVALUES    = "XValues";
    public static final String DATASET_YVALUES    = "YValues";
    public static final String DATASET_ZVALUES    = "ZValues";
    public static final String DATASET_VALUELABEL = "DataSetValueLabel";
    
    //Supported plotter framework
    public static final String FRAMEWORK         = "nl.astron.lofar.sas.plotter.PlotSGTImpl";
    
    //Supported plotter data access class
    //public static final String DATA_ACCESS_CLASS = "nl.astron.lofar.sas.plotter.PlotDataAccessParmDBImpl";
    public static final String DATA_ACCESS_CLASS = "nl.astron.lofar.sas.plotter.test.PlotDataAccessTestImpl";
    
    //Supported plotter data export class
    //public static final String DATA_EXPORT_CLASS = "nl.astron.lofar.sas.plotter.PlotDataExportRootImpl";

    //Exception messages
    public static final String EXCEPTION_GENERIC                = "An internal error occurred with no leads to its cause. Please contact software support.";
    public static final String EXCEPTION_NOT_IMPLEMENTED         = "An internal error occurred : The plotter has detected that the feature you requested is not (fully) implemented in this release. Details: ";
    public static final String EXCEPTION_OPERATION_NOT_SUPPORTED  = "An internal error occurred : The plotter has detected that the operation you requested is not supported by this plotter. Details: ";
    
    public static final String EXCEPTION_EMPTY_DATASET           = "An internal error occurred : An empty data set was passed to the plotter framework.";
    
    public static final String EXCEPTION_FRAMEWORK_INIT          = "An internal error occurred : The plotter framework class specified in the constants file could not be accessed and/or instantiated by the plotter.";
    public static final String EXCEPTION_FRAMEWORK_NOT_FOUND      = "An internal error occurred : The plotter framework class specified in the constants file could not be found by the plotter.";
    public static final String EXCEPTION_FRAMEWORK_NOT_COMPATIBLE = "An internal error occurred : The plotter framework class specified in the constants file does not implement the IPlot interface, and therefore is NOT compatible with the plotter.";
    
    public static final String EXCEPTION_DATA_ACCESS_INIT           = "An internal error occurred : The plotter data access class specified in the constants file could not be accessed and/or instantiated by the plotter.";
    public static final String EXCEPTION_DATA_ACCESS_NOT_FOUND      = "An internal error occurred : The plotter data access class specified in the constants file could not be found by the plotter.";
    public static final String EXCEPTION_DATA_ACCESS_NOT_COMPATIBLE = "An internal error occurred : The plotter data access class specified in the constants file does not implement the IPlotDataAccess interface, and therefore is NOT compatible with the plotter.";
}
