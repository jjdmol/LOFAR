/*
 * PICtableModel.java
 *
 * Created on January 31, 2006, 11:11 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import java.util.Vector;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBvalue;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the PIC table 
 *
 * @author blaakmeer
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
    public boolean fillTable(MainFrame aMainFrame,int aNodeID,String aStartTime, String anEndTime,boolean setMostRecent) {
        
        if (aMainFrame.getSharedVars().getOTDBrmi() == null) {
            logger.debug("No active otdbRmi connection");
            return false;
        }            
        try {
            aMainFrame.getSharedVars().getOTDBrmi().getRemoteValue().setTreeID(aMainFrame.getSharedVars().getTreeID());
            Vector aLogList=aMainFrame.getSharedVars().getOTDBrmi().getRemoteValue().searchInPeriod(aNodeID,1,aStartTime,anEndTime,setMostRecent);
            if (aLogList==null || aLogList.size()<1 ) {
                logger.debug("Failed to get searchInPeriod Match");
                return false;
            }
            data = new Object[aLogList.size()][headers.length];
            for (int k=0; k< aLogList.size();k++) {
               String [] aS=((jOTDBvalue)aLogList.elementAt(k)).name.split("[.]");
               String aName=aS[aS.length-1];
               data[k][0]=aName;
               data[k][1]=((jOTDBvalue)aLogList.elementAt(k)).value;
               data[k][2]=((jOTDBvalue)aLogList.elementAt(k)).time;
            }
            fireTableDataChanged();
        } catch (Exception e) {
            logger.debug("searchInPeriod failed: " + e);
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
    public String getColumnName(int c) {
        try {
            return headers[c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
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
            return null;
        }
    }
}
