/*
 * SubbandFreqTableModel.java
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

import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import org.apache.log4j.Logger;

/**
 * Implements the data model behind the Subband Frequencies table
 *
 * @created 07-06-2006, 13:00
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class SubbandFreqTableModel extends javax.swing.table.AbstractTableModel {
    
    private String headers[] = {"Subband #","Frequency"};
    private Object data[][];
    private MainFrame itsMainFrame;
    
    static Logger logger = Logger.getLogger(SubbandFreqTableModel.class);
    static String name = "SubbandFreqTableModel";

    /** Creates a new instance of SubbandFreqTableModel */
    public SubbandFreqTableModel(MainFrame aMainFrame) {
        data = new Object[0][0]; 
        itsMainFrame=aMainFrame;
    }
    
    
    /** Fills the table from the database */
    public boolean fillTable(MainFrame aMainFrame) {
        
        if (SharedVars.getOTDBrmi() == null) {
            logger.debug("No active otdbRmi connection");
            return false;
        }            
        try {
//            data = new Object[aLogList.size()][headers.length];
//            for (int k=0; k< aLogList.size();k++) {
//               String [] aS=((jOTDBvalue)aLogList.elementAt(k)).name.split("[.]");
//               String aName=aS[aS.length-1];
//               data[k][0]=aName;
//               data[k][1]=((jOTDBvalue)aLogList.elementAt(k)).value;
//               data[k][2]=((jOTDBvalue)aLogList.elementAt(k)).time;
//               data[k][3]=((jOTDBvalue)aLogList.elementAt(k)).time;
//            }
            fireTableDataChanged();
        } catch (Exception e) {
            logger.debug("filling table failed: " + e);
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
