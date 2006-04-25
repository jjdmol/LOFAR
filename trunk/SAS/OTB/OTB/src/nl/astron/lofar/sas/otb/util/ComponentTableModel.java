/*
 * ComponentTableModel.java
 *
 * Created on April 4th, 2006, 15:28
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the component table 
 *
 * @author coolen
 */
public class ComponentTableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"nodeID","Name","Version","Classification","Constraints","Description"};
    private OtdbRmi otdbRmi;
    private Object data[][];
    
    static Logger logger = Logger.getLogger(ComponentTableModel.class);
    static String name = "ComponentTableModel";

    /** Creates a new instance of ComponentTableModel */
    public ComponentTableModel(OtdbRmi otdbRmi) {

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
            if (! otdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }

            // get TreeID that needs 2b refreshed
            int aNodeID=((Integer)data[row][0]).intValue();
            jVICnodeDef tInfo=otdbRmi.getRemoteMaintenance().getComponentNode(aNodeID);
            if ( tInfo == null) {
                logger.debug("Unable to get ComponentInfo for node with ID: " + aNodeID);
                return false;
            }
            data[row][0]=new Integer(tInfo.nodeID());	   
            data[row][1]=new String(tInfo.name);
	    data[row][2]=new Integer(tInfo.version);
            data[row][3]=new String(otdbRmi.getClassif().get(tInfo.classif));
	    data[row][4]=new String(tInfo.constraints);
	    data[row][5]=new String(tInfo.description);
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
            if (otdbRmi.getRemoteOTDB() != null && ! otdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }
            // Get a list of all available Components (topnode)
            Vector aComponentList=otdbRmi.getRemoteMaintenance().getComponentList("%",false);
            data = new Object[aComponentList.size()][headers.length];
            logger.debug("Componentlist downloaded. Size: "+aComponentList.size());
           
            for (int k=0; k< aComponentList.size();k++) {
                jVICnodeDef tInfo = (jVICnodeDef)aComponentList.elementAt(k);
                if (tInfo == null) {
                    logger.debug("No such component found!");
                } else {
                    logger.debug("Gathered info for ID: "+tInfo.nodeID());
                    data[k][0]=new Integer(tInfo.nodeID());	   
	            data[k][1]=new String(tInfo.name);
	            data[k][2]=new Integer(tInfo.version);
                    data[k][3]=new String(otdbRmi.getClassif().get(tInfo.classif));
	            data[k][4]=new String(tInfo.constraints);
	            data[k][5]=new String(tInfo.description);
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
