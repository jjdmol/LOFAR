/*
 * BeamformerConfigurationTableModel.java
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

 /**
 * Implements the data model behind the BeamConfiguration table
 *
 * @created 06-10-2008, 12:00
 *
 *
 * @version $Id$
 *
 * @updated
 */

package nl.astron.lofar.sas.otb.util.tablemodels;

import java.util.Vector;
import org.apache.log4j.Logger;

/**
 *
 * @author Coolen
 */
public class BeamformerConfigurationTableModel extends javax.swing.table.DefaultTableModel {
   
    static Logger logger = Logger.getLogger(BeamformerConfigurationTableModel.class);
    static String name = "BeamConfigurationTableModel";

    private String itsTreeType;    
    
    /** Creates a new instance of BeamConfigurationTableModel */
    public BeamformerConfigurationTableModel() { 
        this.addColumn("Beamformer");
        this.addColumn("Stations");
    }
    
    /** fills the table with the initial settings
     *
     * @param  stations   Vector<String> of all stations for this beamformer
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType, Vector<String> stations) {
         
        itsTreeType=treeType;
        
        // "clear" the table
        setRowCount(0);
        if (stations==null) {
            logger.error("Error in fillTable, null value in input found.");
            return false;
        }
        int length = stations.size();
        
        // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        int offset=1;
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }
        
        // need to skip first entry because it is the default (dummy) TBBsetting
        for (int i=0; i<length-offset; i++) {
            String[]  newRow = { Integer.toString(i+1),stations.elementAt(i+offset)};
            
            this.addRow(newRow);
        }
        fireTableDataChanged();
        return true;    
    }
 
    /** fills the table with the initial settings
     *
     * @param  stations    Vector<String> of all direction stations
     *
     * @return True if succes else False
     */
     public boolean getTable(Vector<String> stations) {
         
        int length = stations.size();
        
        // need to skip first entry because it is the default (dummy) TBBsetting
        // empty all elements except the default
        stations.setSize(1);
        
        for (int i=0; i<getRowCount(); i++) {
            stations.addElement(getValueAt(i,1).toString());
        }
        return true;    
    }
     
     
    /**  Add an entry to the tableModel
     *
     * @param  stations   direction Type
     *
     * @return True if succes else False
     */
    public boolean addRow(String stations) {
      
        if (stations==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }
        String[]  newRow = { Integer.toString(this.getRowCount()+1),stations};
        this.addRow(newRow);
        
        return true;
    }
    
    /** Update a row with new information
     *
     * @param   newRow  String[] that contains all values as they should be for this row
     * @param   row     int with the rownumber.
     */
    public boolean updateRow(String stations,int row) {
        if (row < this.getRowCount() && row >= 0) {
            this.setValueAt(stations,row,1);
        } else {
            logger.error("Error in updateRow, illegal rownumber supplied");
            return false;
        }
        fireTableDataChanged();
        return true;
    }
    
    /** get the values from the given row
     *
     * @param   row int with row number
     *
     * @return  String containing all values from the given row
     */
    public String getSelection(int row) {
        if (row < this.getRowCount() && row >= 0) {
            String selection = (String)this.getValueAt(row,1);
            return selection;
        } else {
            return null;
        }
                               
    }
    

    /** returns the isEditable flag from the given row and column.
     *  we need to override this method, since originally all ros/colums from the DefaultTableModel are editable
     *
     * @param   row     rownumber
     * @param   column  columnnumber
     *
     * @return  true if the asked cell is editable
     */
    @Override
    public boolean isCellEditable(int row, int column) {
        return false;
    }


}
