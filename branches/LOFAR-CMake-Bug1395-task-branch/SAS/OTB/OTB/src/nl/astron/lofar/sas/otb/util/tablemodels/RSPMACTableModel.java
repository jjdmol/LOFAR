/*
 * RSPMACTableModel.java
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

import org.apache.log4j.Logger;

/**
 * Implements the data model behind the RSP board MACAddresses table
 *
 * @created 07-06-2006, 13:30
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class RSPMACTableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"RSPBoard#","MAC address Sender","MAC address Receiver"};
    private Object data[][];
    
    static Logger logger = Logger.getLogger(RSPMACTableModel.class);
    static String name = "RSPMACTableModel";

    /** Creates a new instance of RSPMACTableModel */
    public RSPMACTableModel() {
        data = new Object[0][0];
    }
    
    
    /** Fills the table */
    public boolean fillTable(String aNameList,String aList) {
        if (aNameList.length()<1 || aList.length() <1) {
            logger.error("No data to make a table from");
            return false;
        }
        
        aNameList = aNameList.replaceAll("[\\[\\]]","");
        aList = aList.replaceAll("[\\[\\]]","");
        
        String aNS[]=aNameList.split(",");
        String aS[]=aList.split(",");
        
        data = new Object[aNS.length][headers.length];

        int i=0;
        for (int k=0;k<aNS.length;k++) {
            data[k][0]= aNS[k];
            if (i < aS.length) {
                data[k][1]= aS[i];            
            } else {
                logger.debug("Provided RSPMACAddress string to short, adding empty strings");
                data[k][1] = "";
            }
            if (i+1 < aS.length) {
                data[k][2]= aS[i+1];
            } else {
                logger.debug("Provided RSPMACAddress string to short, adding empty strings");
                data[k][2] = "";                
            }
            i+=2;
        }
        fireTableDataChanged();
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
