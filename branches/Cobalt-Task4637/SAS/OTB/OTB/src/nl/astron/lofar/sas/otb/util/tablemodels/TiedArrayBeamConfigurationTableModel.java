/*
 * TiedArrayBeamConfigurationTableModel.java
 *
 *  Copyright (C) 2002-2010
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
 * Implements the data model behind the TiedArrayBeamConfiguration table
 *
 * @created 10-12-2011, 15:15
 *
 *
 * @version $Id:$
 *
 * @updated
 */

package nl.astron.lofar.sas.otb.util.tablemodels;

import java.util.ArrayList;
import nl.astron.lofar.sas.otb.objects.TiedArrayBeam;
import org.apache.log4j.Logger;

/**
 *
 * @author Coolen
 */
public class TiedArrayBeamConfigurationTableModel extends javax.swing.table.DefaultTableModel {
   
    static Logger logger = Logger.getLogger(TiedArrayBeamConfigurationTableModel.class);
    static String name = "TiedArrayBeamConfigurationTableModel";

    private ArrayList<TiedArrayBeam> itsTiedArrayBeams = new ArrayList<>();

    private int offset=1;
    private String itsTreeType=null;

    private boolean isChanged=false;

    
    /** Creates a new instance of BeamConfigurationTableModel */
    public TiedArrayBeamConfigurationTableModel() { 
        this.addColumn("dirtype");
        this.addColumn("angle 1");
        this.addColumn("angle 2");
        this.addColumn("coordtype");
        this.addColumn("disp.Measure");
        this.addColumn("coherent");
    }
    
    /** fills the table with the initial settings
     *
     * @param treeType    VIC or Template
     * @param  refill     false for initial fill, true for refill
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType ,ArrayList<TiedArrayBeam> aTiedArrayBeamList,boolean refill) {
         
        // "clear" the table
        setRowCount(0);
        if (aTiedArrayBeamList==null) {
            logger.error("Error in fillTable, null value in input found.");
            return false;
        }

        
        removeAllRows();

        itsTreeType = treeType;

        ArrayList<TiedArrayBeam> testList = new ArrayList<>(aTiedArrayBeamList);
        
                // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }


        // need to skip first entry because it is the default (dummy) TABsetting
        boolean skip = false;
        if (offset!=0) {
            skip = true;
        }
        for (TiedArrayBeam b : testList ) {
            if (skip) {
                skip = false;
                itsTiedArrayBeams.add(b);
                continue;
            }
            this.addRow(b);
        }

        isChanged=refill;
        fireTableDataChanged();
        return true;    
    }
 
    /** fills the table with the initial settings
     *
     *
     * @return True if succes else False
     */
     public ArrayList<TiedArrayBeam> getTable() {
         return itsTiedArrayBeams;    
    }
     
     
    /**  Add an entry to the tableModel
     *
     *
     * @return True if succes else False
     */
    public boolean addRow(TiedArrayBeam aTiedArrayBeam) {
      
        if (aTiedArrayBeam==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }

        itsTiedArrayBeams.add(aTiedArrayBeam);

        String[]  newRow = { aTiedArrayBeam.getDirectionType(),
                             aTiedArrayBeam.getAngle1(),
                             aTiedArrayBeam.getAngle2(),
                             aTiedArrayBeam.getCoordType(),
                             aTiedArrayBeam.getDispersionMeasure(),
                             aTiedArrayBeam.getCoherent()};
        this.addRow(newRow);
        itsTiedArrayBeams.trimToSize();

        isChanged=true;
        return true;
    }
    
    /** Update a row with new information
     *
     * @param   newRow  String[] that contains all values as they should be for this row
     * @param   row     int with the rownumber.
     */
    public boolean updateRow(TiedArrayBeam aNewTiedArrayBeam,int row) {
        if (row < this.getRowCount() && row >= 0) {
            if (itsTiedArrayBeams != null) {
                itsTiedArrayBeams.set(row+offset, aNewTiedArrayBeam);
            itsTiedArrayBeams.trimToSize();
            } else {
                return false;
            }
        } else {
            logger.error("Error in updateRow, illegal rownumber supplied");
            return false;
        }
        isChanged=true;
        
        this.setValueAt(aNewTiedArrayBeam.getDirectionType(),row,0);
        this.setValueAt(aNewTiedArrayBeam.getAngle1(),row,1);
        this.setValueAt(aNewTiedArrayBeam.getAngle2(),row,2);
        this.setValueAt(aNewTiedArrayBeam.getCoordType(),row,3);
        this.setValueAt(aNewTiedArrayBeam.getDispersionMeasure(),row,4);
        this.setValueAt(aNewTiedArrayBeam.getCoherent(),row,5);


        fireTableDataChanged();
        return true;
    }
    
    /** get the values from the given row
     *
     * @param   row int with row number
     *
     * @return  String[] containing all values from the given row
     */
    public TiedArrayBeam getSelection(int row) {
        if (row < this.getRowCount() && row >= 0) {
            if (itsTiedArrayBeams != null) {
                return itsTiedArrayBeams.get(row+offset);
            } else {
                return null;
            }
        } else {
            return null;
        }
                               
    }

    public void removeAllRows() {
        this.setRowCount(0);
        if (itsTiedArrayBeams != null) {
            itsTiedArrayBeams.clear();
            itsTiedArrayBeams.trimToSize();
        }
        isChanged=true;
    }



    @Override
    public void removeRow(int row) {
        super.removeRow(row);
        if (itsTiedArrayBeams != null) {
            itsTiedArrayBeams.remove(row+offset);
            itsTiedArrayBeams.trimToSize();
        }
        isChanged=true;
    }

    public boolean changed() {
        return isChanged;
    }

    @Override
    public Class getColumnClass(int c) {
        Object value=this.getValueAt(0,c);
        return (value==null?Object.class:value.getClass());
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