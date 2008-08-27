/*
 * PlotConstants.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl

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
 */

package nl.astron.lofar.java.gui.plotter;

/**
 * This abstract class serves the plotter by storing strings often used in a central
 * and easy to control location. It contains the:<br><br>
 * - Data model constants (DATASET_*) used by both the Data Access/Export and the Plot framework<br>
 * - Plot types (PLOT_*) used to centralize the plotter's capabilities<br>
 * - Plotter Configuration file pointer (RESOURCE_FILE) used to determine the classes to be used for data access/export and graphics plotting<br>
 * - Exception strings (EXCEPTION_*) that come in handy throughout all classes. Centralized to make easy maintenance.<br>
 *
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
    
    public static final String DATASET_AXIS_TYPE_SPATIAL = "SPATIAL";    
    public static final String DATASET_AXIS_TYPE_MJDTIME = "MJDTIME";
    public static final String DATASET_AXIS_TYPE_TIME = "TIME";
    public static final String DATASET_AXIS_TYPE_LOG = "LOGARITHMIC";
    
    //Data Model operator identifiers for updating datasets
    
    public static final String DATASET_OPERATOR_ADD = "DATASETADD";
    public static final String DATASET_OPERATOR_MODIFY = "DATASETMODIFY";
    public static final String DATASET_OPERATOR_DELETE = "DATASETDELETE";
    public static final String DATASET_OPERATOR_UPDATE = "DATASETUPDATE";
    
    //Identifiers for plot types
    public static final int PLOT_BAR = 1;
    public static final int PLOT_XYLINE = 2;
    public static final int PLOT_POINTS = 3;
    public static final int PLOT_SCATTER = 4;
    public static final int PLOT_GRID = 5;
        
    //Package-like Location of the properties file (minus .properties!) where the data plot/access/export classes are specified
    public static final String RESOURCE_FILE     = "plotter_config";
    
    //Exception messages
    public static final String EXCEPTION_GENERIC                    = "An error occurred! ";
    public static final String EXCEPTION_NOT_IMPLEMENTED            = "The feature you requested is not (fully) implemented in this release. ";
    public static final String EXCEPTION_OPERATION_NOT_SUPPORTED    = "The operation you requested is not supported. ";
    
    public static final String EXCEPTION_EMPTY_DATASET              = "An empty data set was passed to the plotter framework. ";
    public static final String EXCEPTION_INVALID_DATASET            = "An invalid or incomplete data set was passed to the plotter framework. ";
  
    public static final String EXCEPTION_RESOURCE_FILE_NOT_FOUND    = "The plotter_config.properties file could not be found by the plotter. Please check your classpath and the location of the file.";
            
    public static final String EXCEPTION_FRAMEWORK_INIT             = "The plotter framework class specified in the plotter_config.properties file could not be accessed and/or instantiated by the plotter. ";
    public static final String EXCEPTION_FRAMEWORK_NOT_FOUND        = "The plotter framework class specified in the plotter_config.properties file could not be found by the plotter. ";
    public static final String EXCEPTION_FRAMEWORK_NOT_COMPATIBLE   = "The plotter framework class specified in the plotter_config.properties file does not implement the IPlot interface, and therefore is NOT compatible with the plotter. ";
    
    public static final String EXCEPTION_DATA_ACCESS_INIT           = "The plotter data access class specified in the plotter_config.properties file could not be accessed and/or instantiated by the plotter. ";
    public static final String EXCEPTION_DATA_ACCESS_NOT_FOUND      = "The plotter data access class specified in the plotter_config.properties file could not be found by the plotter. ";
    public static final String EXCEPTION_DATA_ACCESS_NOT_COMPATIBLE = "The plotter data access class specified in the plotter_config.properties file does not implement the IPlotDataAccess interface, and therefore is NOT compatible with the plotter. ";

}
