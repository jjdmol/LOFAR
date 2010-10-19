/*
 * DefaultTemplatetableModel.java
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

import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jDefaultTemplate;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.*;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the DefaultTemplate table
 *
 * @created 04-05-2010, 11:11
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class DefaultTemplatetableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"ID","Name","OriginalTree","Status","Classification","Campaign","MoMID","Description"};
    private OtdbRmi otdbRmi;
    private Object data[][];

    static Logger logger = Logger.getLogger(DefaultTemplatetableModel.class);
    static String name = "DefaultTemplatetableModel";
    
    /** Creates a new instance of PICtableModel */
    public DefaultTemplatetableModel(OtdbRmi otdbRmi) {
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
                logger.error("No open connection available");
                return false;
            }

            // get TreeID that needs 2b refreshed
            // first get all defaulttemplates from the database
            Vector<jDefaultTemplate> templateList= OtdbRmi.getRemoteOTDB().getDefaultTemplates();
            Iterator<jDefaultTemplate> it = templateList.iterator();
            int aTreeID=((Integer)data[row][0]).intValue();
            jDefaultTemplate aDF=null;
            while(it.hasNext()) {
                aDF = it.next();
                if (aDF.treeID() == aTreeID) break;
            }

            // if no match, return
            if (aDF == null) {
                logger.warn("Couldn't find Matching DefaultTemplate in database");
                return false;
            }

            jOTDBtree tInfo=OtdbRmi.getRemoteOTDB().getTreeInfo(aTreeID, false);
            if ( tInfo == null) {
                logger.warn("Unable to get treeInfo for tree with ID: " + aTreeID);
                return false;
            }
            data[row][0]=new Integer(tInfo.treeID());
            data[row][1]=new String(aDF.name);
            data[row][2]=new Integer(tInfo.originalTree);
            data[row][3]=new String(OtdbRmi.getTreeState().get(tInfo.state));
            data[row][4]=new String(OtdbRmi.getClassif().get(tInfo.classification));
            data[row][5]=new String(tInfo.campaign);
            data[row][6]=new Integer(tInfo.momID());
            data[row][7]=new String(tInfo.description);
            fireTableDataChanged();
        } catch (RemoteException e) {
            logger.debug("Remote OTDB via RMI and JNI failed: " + e);
        } 
        return true;
    }
    
    /** Fills the table from the database */
    public boolean fillTable() {
       if (otdbRmi == null) {
            logger.warn("No active otdbRmi connection");
            return false;
       }
       try {
            if (OtdbRmi.getRemoteOTDB() != null && ! OtdbRmi.getRemoteOTDB().isConnected()) {
                logger.error("No open connection available");
                return false;
            }
            // Get a Treelist of all available VItemplate's
            Vector<jDefaultTemplate> aTreeList=OtdbRmi.getRemoteOTDB().getDefaultTemplates();
            Iterator<jDefaultTemplate> it = aTreeList.iterator();
            data = new Object[aTreeList.size()][headers.length];
            logger.debug("DefaultTreelist downloaded. Size: "+aTreeList.size());
            int k=0;
            while (it.hasNext()) {
                jDefaultTemplate aDF = it.next();
                jOTDBtree tInfo =OtdbRmi.getRemoteOTDB().getTreeInfo(aDF.treeID(), false);
                if (tInfo.treeID()==0) {
                    logger.warn("Illegal TreeID found!");
                } else {
                    logger.debug("Gathered info for ID: "+tInfo.treeID());
                    data[k][0]=new Integer(tInfo.treeID());
                    data[k][1]=new String(aDF.name);
                    data[k][2]=new Integer(tInfo.originalTree);
	            data[k][3]=new String(OtdbRmi.getTreeState().get(tInfo.state));
                    data[k][4]=new String(OtdbRmi.getClassif().get(tInfo.classification));
	            data[k][5]=new String(tInfo.campaign);
	            data[k][6]=new Integer(tInfo.momID());
	            data[k][7]=new String(tInfo.description);
                    k++;
                }
            }
            fireTableDataChanged();
        } catch (RemoteException e) {
            logger.error("Remote OTDB via RMI and JNI failed: " + e);
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

    @Override
    public Class getColumnClass(int c) {
        return getValueAt(0, c).getClass();
    }
}
