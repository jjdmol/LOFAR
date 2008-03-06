/*
 * ComponentTableModel.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.*;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the component table 
 *
 * @created 04-04-2006, 15:28
 * @author coolen
 * @version $Id$
 * @updated
 */
public class ComponentTableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"ID","Name","Version","Classification","Constraints","Description"};
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
            if (! OtdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }

            // get TreeID that needs 2b refreshed
            int aNodeID=((Integer)data[row][0]).intValue();
            jVICnodeDef tInfo=OtdbRmi.getRemoteMaintenance().getComponentNode(aNodeID);
            if ( tInfo == null) {
                logger.debug("Unable to get ComponentInfo for node with ID: " + aNodeID);
                return false;
            }
            data[row][0]=new Integer(tInfo.nodeID());	   
            data[row][1]=new String(tInfo.name);
	    data[row][2]=new Integer(tInfo.version);
            data[row][3]=new String(OtdbRmi.getClassif().get(tInfo.classif));
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
            if (OtdbRmi.getRemoteOTDB() != null && ! OtdbRmi.getRemoteOTDB().isConnected()) {
                logger.debug("No open connection available");
                return false;
            }
            // Get a list of all available Components (topnode)
            Vector aComponentList=OtdbRmi.getRemoteMaintenance().getComponentList("%",false);
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
                    data[k][3]=new String(OtdbRmi.getClassif().get(tInfo.classif));
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
