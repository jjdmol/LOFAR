/*
 * PICtableModel.java
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
import javax.swing.JFileChooser;
import nl.astron.lofar.sas.otb.util.*;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the PIC table 
 *
 * @created 31-01-2006, 11:11
 *
 * @author blaakmeer
 *
 * @version $Id: PICtableModel.java 16356 2010-09-21 11:58:12Z coolen $
 *
 * @updated
 */
public final class MetadataTableModel extends javax.swing.table.DefaultTableModel {
    
    private String  headers[] = {"Parameter","value"};
    private OtdbRmi otdbRmi;
    private Object  data[][];
    private int     itsTreeID;
    private JFileChooser fc          = null;
    private String itsUser ="";

    
    static Logger logger = Logger.getLogger(MetadataTableModel.class);
    static String name = "StateChangeHistoryTableModel";

    /** Creates a new instance of PICtableModel */
    public MetadataTableModel(OtdbRmi otdbRmi,int treeID, String aUser) {

        this.otdbRmi = otdbRmi;
        this.itsTreeID=treeID;
        this.itsUser=aUser;
        boolean fillTable = fillTable();
    }

    public void setTree(int treeID) {
        this.itsTreeID=treeID;
        fillTable();
    }
    
    
    
  /** Fills the table from the database */
    public boolean fillTable() {
        
       if (otdbRmi == null) {
            logger.error("No active otdbRmi connection");
            return false;
        }
        try {
            if (OtdbRmi.getRemoteOTDB() != null && ! OtdbRmi.getRemoteOTDB().isConnected()) {
                logger.error("No open connection available");
                return false;
            }
            data=null;
            this.setRowCount(0);

            // create filename that can be used at the remote site    
            String aRemoteFileName="/tmp/"+itsTreeID+"_"+itsUser+".Metadata";
                    
            // write the parset
            OtdbRmi.getRemoteMaintenance().exportMetadata(itsTreeID,aRemoteFileName); 
                    
            //obtain the remote file
            byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
             
            String aParSet=new String(dldata);
            
            // split inputfile in different lines on return.
            String[] lines = aParSet.split("\n");

            data = new Object[lines.length][headers.length];
            logger.debug("metadata downloaded. Size: "+lines.length);
           
            for (int k=0; k< lines.length;k++) {
                String[] keyval = lines[k].split("=");
                if (keyval.length >= 1 ) data[k][0]=keyval[0];
                if (keyval.length >= 2 ) data[k][1]=keyval[1];
            }
            fireTableDataChanged();
        } catch (RemoteException e) {
            logger.debug("Remote OTDB getMetadata failed: " + e);
	} 
        return true;
    }
    public int getTreeID() {
        return itsTreeID;
    }

    /** Returns the number of rows 
     *  @return Nr of rows 
     */
    @Override
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
            logger.error("ArrayIndex out of bound exception for getColumnName("+c+"): "+e);
            return null;
        }
        
    }

    /** Returns the number of columns 
     * @return  The number of columns
     */
    @Override
    public int getColumnCount() {
        return headers.length;
    }

    /** Returns value from table
     * @param    r   rownumber
     * @param    c   columnnumber
     * 
     * @return  the value at row,column
     */
    @Override
    public Object getValueAt(int r, int c) {
        try {
            return data[r][c];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            logger.error("ArrayIndex out of bound exception for getValueAt("+r+","+c+"): "+e);
            return null;
        }
    }

    @Override
    public void setValueAt(Object value, int row, int col) {
        data[row][col] = value;
        fireTableCellUpdated(row, col);
    }


    @Override
    public Class getColumnClass(int c) {
        Object value=this.getValueAt(0,c);
        return (value==null?Object.class:value.getClass());
    }
}
