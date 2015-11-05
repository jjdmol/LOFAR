/*
 * LogParamTableModel.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
 */

package nl.astron.lofar.sas.otb.util.tablemodels;

import java.rmi.RemoteException;
import java.util.ArrayList;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBvalue;
import nl.astron.lofar.sas.otb.util.*;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the LogParam table 
 *
 * @created 31-01-2006, 11:11
 *
 * @author Coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class LogParamTableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"Parameter","Value","TimeStamp"};
    private Object data[][];
    
    static Logger logger = Logger.getLogger(LogParamTableModel.class);
    static String name = "LogParamTableModel";

    /** Creates a new instance of PICtableModel */
    public LogParamTableModel() {
        data = new Object[0][0];
    }
    
    
    /** Fills the table from the database */
    public boolean fillTable(MainFrame aMainFrame,int aNodeID) {
        
        if (SharedVars.getOTDBrmi() == null) {
            logger.error("No active otdbRmi connection");
            return false;
        }            
        try {
            OtdbRmi.getRemoteValue().setTreeID(aMainFrame.getSharedVars().getTreeID());
            ArrayList<jOTDBvalue> aLogList=new ArrayList<>(OtdbRmi.getRemoteValue().searchInPeriod(aNodeID,
                    aMainFrame.getSharedVars().getLogParamLevel(),
                    aMainFrame.getSharedVars().getLogParamStartTime(),
                    aMainFrame.getSharedVars().getLogParamEndTime(),
                    aMainFrame.getSharedVars().getLogParamMostRecent()));
            if (aLogList==null || aLogList.size()<1 ) {
                logger.warn("No matches for this searchInPeriod");
                return true;
            }
            data = new Object[aLogList.size()][headers.length];
            int k=0;
            for (jOTDBvalue log:aLogList) {
               data[k][0]=log.name;
               data[k][1]=log.value;
               data[k][2]=log.time;
               k++;
            }
            fireTableDataChanged();
        } catch (RemoteException e) {
            logger.error("Remote exception on searchInPeriod: " + e);
            return false;
        } 
       

        return true;
    }

    /** Returns the number of rows 
     *  @return Nr of rows 
     */
    public int getRowCount() {
        return data.length;
    }

    /** Returns the column names 
     * @param    c   Column Number
     * @return  the name for this column     
     */
    @Override
    public String getColumnName(int c) {
        try {
            return headers[c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            logger.error("Array index out of bounds in getColumn("+c+"): "+e);
            return null;
        }
        
    }

    /** Returns the number of columns 
     * @return  The number of columns
     */
    public int getColumnCount() {
        return headers.length;
    }

    /** Returns value from table
     * @param    r   rownumber
     * @param    c   columnnumber
     * 
     * @return  the value at row,column
     */
    public Object getValueAt(int r, int c) {
        try {
            return data[r][c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            logger.error("Array index out of bounds in getValueAt("+r+","+c+"): "+e);
            return null;
        }
    }

    @Override
    public Class getColumnClass(int c) {
        Object value=this.getValueAt(0,c);
        return (value==null?Object.class:value.getClass());
    }
}
