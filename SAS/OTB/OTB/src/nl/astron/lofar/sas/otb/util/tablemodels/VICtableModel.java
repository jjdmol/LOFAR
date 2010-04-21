/*
 * VICtableModel.java
 *
 * Created on January 31, 2006, 11:11 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util.tablemodels;

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.*;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the VIC table 
 *
 * @created 31-01-2006, 11:11
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class VICtableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"ID","OriginalTree","Status","Campaign","MoMID","StartTime","StopTime","Description"};
    private OtdbRmi otdbRmi;
    private Object data[][];

    static Logger logger = Logger.getLogger(VICtableModel.class);
    static String name = "VICtableModel";
   
    /** Creates a new instance of PICtableModel */
    public VICtableModel(OtdbRmi otdbRmi) {
        this.otdbRmi = otdbRmi;
        fillTable();
    }

    /** Refreshes 1 row from table out of the database
     *
     * @param   row     Rownr that needs to be refreshed
     * @return  suceeded or failed status
     */ 
    public boolean refreshRow(int row) {
        
        try {
            if (! OtdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }

            // get TreeID that needs 2b refreshed
            int aTreeID=((Integer)data[row][0]).intValue();
            jOTDBtree tInfo=OtdbRmi.getRemoteOTDB().getTreeInfo(aTreeID, false);
            if ( tInfo == null) {
                logger.debug("Unable to get treeInfo for tree with ID: " + aTreeID);
                return false;
            }
            data[row][0]=new Integer(tInfo.treeID());	   
            data[row][1]=new Integer(tInfo.originalTree);	   
            data[row][2]=new String(OtdbRmi.getTreeState().get(tInfo.state));
            data[row][3]=new String(tInfo.campaign);
            data[row][4]=new Integer(tInfo.momID());
            data[row][5]=new String(tInfo.starttime.replace("T", " "));
            data[row][6]=new String(tInfo.stoptime.replace("T", " "));
            data[row][7]=new String(tInfo.description);
            fireTableDataChanged();
        } catch (Exception e) {
            logger.debug("Remote OTDB via RMI and JNI failed: " + e);
        } 
        return true;
    }
    
    /** Fills the table from the database */
    public boolean fillTable() {
       if (otdbRmi == null) {
            logger.debug("No active otdbRmi connection");
            return false;
        }        
        try {
            if (OtdbRmi.getRemoteOTDB() != null && ! OtdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }
            // Get a Treelist of all available VHtree's
            Vector aTreeList=OtdbRmi.getRemoteOTDB().getTreeList(OtdbRmi.getRemoteTypes().getTreeType("VHtree"),(short)0);
            data = new Object[aTreeList.size()][headers.length];
            logger.debug("Treelist downloaded. Size: "+aTreeList.size());
           
            for (int k=0; k< aTreeList.size();k++) {
                jOTDBtree tInfo = (jOTDBtree)aTreeList.elementAt(k);
                if (tInfo.treeID()==0) {
                    logger.debug("No such tree found!");
                } else {
                    logger.debug("Gathered info for ID: "+tInfo.treeID());
                    data[k][0]=new Integer(tInfo.treeID());	   
                    data[k][1]=new Integer(tInfo.originalTree);	   
	            data[k][2]=new String(OtdbRmi.getTreeState().get(tInfo.state));
	            data[k][3]=new String(tInfo.campaign);
	            data[k][4]=new Integer(tInfo.momID());
	            data[k][5]=new String(tInfo.starttime.replace("T", " "));
	            data[k][6]=new String(tInfo.stoptime.replace("T", " "));
	            data[k][7]=new String(tInfo.description);
                }
            }
            fireTableDataChanged();
        } catch (Exception e) {
            logger.debug("Remote OTDB via RMI and JNI failed: " + e);
	} 
        return true;
    }
    
    /** Returns the number of rows 
     *  @return Nr of rows 
     */
    public int getRowCount() {
        if (data != null) {
            return data.length;
        } else {
            return 0;
        }
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
