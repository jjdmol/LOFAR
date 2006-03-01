/*
 * VICtableModel.java
 *
 * Created on January 31, 2006, 11:11 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the PIC table 
 *
 * @author coolen
 */
public class VICtableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"TreeID","OriginalTree","Status","Campaign","MoMID","StartTime","StopTime","Description"};
    private OtdbRmi otdbRmi;
    private Object data[][];

    static Logger logger = Logger.getLogger(VICtableModel.class);
    static String name = "VICtableModel";
   
    /** Creates a new instance of PICtableModel */
    public VICtableModel(OtdbRmi otdbRmi) {
        this.otdbRmi = otdbRmi;
        fillTable();
    }

    /** Fills the table from the database */
    public boolean fillTable() {
        
        try {
            if (! otdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }
            // Get a Treelist of all available VHtree's
            Vector aTreeList=otdbRmi.getRemoteOTDB().getTreeList(otdbRmi.getRemoteTypes().getTreeType("VHtree"),(short)0);
            data = new Object[aTreeList.size()][headers.length];
            logger.debug("Treelist downloaded. Size: "+aTreeList.size());
           
            for (int k=0; k< aTreeList.size();k++) {
                jOTDBtree tInfo = (jOTDBtree)aTreeList.elementAt(1);
                if (tInfo.treeID()==0) {
                    logger.debug("No such tree found!");
                } else {
                    logger.debug("Gathered info for ID: "+tInfo.treeID());
                    data[k][0]=new Integer(tInfo.treeID());	   
                    data[k][1]=new Integer(tInfo.originalTree);	   
	            data[k][2]=new String(otdbRmi.getTreeState().get(tInfo.state));
	            data[k][3]=new String(tInfo.campaign);
	            data[k][4]=new Integer(tInfo.momID);
	            data[k][5]=new String(tInfo.starttime);
	            data[k][6]=new String(tInfo.stoptime);
//	            data[k][7]=new String(tInfo.description);
                }
            }
            fireTableDataChanged();
        } catch (Exception e) {
            logger.debug("Remote OTDB via RMI and JNI failed: " + e);
	} 
        return true;
    }
    
    /** Returns the number of rows */
    public int getRowCount() {
        int rowCount;
        
        //TODO: get rowcount from OTDB??
        rowCount = data.length;
        return rowCount;
    }

    /** Returns the column names */
    public String getColumnName(int c) {
        try {
            return headers[c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }

    /** Returns the number of columns */
    public int getColumnCount() {
        return headers.length;
    }

    /** Returns the value at row r and column c */
    public Object getValueAt(int r, int c) {
        try {
            return data[r][c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }
}
