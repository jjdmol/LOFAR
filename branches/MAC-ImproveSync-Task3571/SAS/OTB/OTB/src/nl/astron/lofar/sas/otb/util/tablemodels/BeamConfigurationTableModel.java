/*
 * BeamConfigurationTableModel.java
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
 * @created 11-02-2008, 13:30
 *
 *
 * @version $Id$
 *
 * @updated
 */

package nl.astron.lofar.sas.otb.util.tablemodels;

import java.util.ArrayList;
import nl.astron.lofar.sas.otb.objects.Beam;
import org.apache.log4j.Logger;

/**
 *
 * @author Coolen
 */
public class BeamConfigurationTableModel extends javax.swing.table.DefaultTableModel {
   
    static Logger logger = Logger.getLogger(BeamConfigurationTableModel.class);
    static String name = "BeamConfigurationTableModel";

    private String itsTreeType = null;
    private ArrayList<Beam> itsBeams = new ArrayList<>();

    private int offset=1;

    private boolean isChanged=false;

    /** Creates a new instance of BeamConfigurationTableModel */
    public BeamConfigurationTableModel() { 
        this.addColumn("dirtype");
        this.addColumn("angle 1");
        this.addColumn("angle 2");
        this.addColumn("coordtype");
        this.addColumn("#TAB");
        this.addColumn("subbands");
        this.addColumn("beamlets");
    }
    
    /** fills the table with the initial settings
     *
     * @param  refill     false for initial fill, true for refill
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType,ArrayList<Beam> aBeamList ,boolean refill){

        if (aBeamList == null) {
            logger.error("Error in fillTable, null value in input found.");
            return false;
        }
        itsTreeType=treeType;
        
        // "clear" the table
        setRowCount(0);
        //empty old settings
        removeAllRows();
        
        // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }
        
        // add all objects to table
        ArrayList<Beam> testList = new ArrayList<>(aBeamList);
        
        boolean skip = false;
        if (offset!=0) {
            skip = true;
        }
        for (Beam b : testList ) {
            if (skip) {
                skip = false;
                itsBeams.add(b);
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
     * @param  aDirTypes   Vector<String> of all direction Types
     *
     * @return True if succes else False
     */
     public ArrayList<Beam> getTable() {
        return itsBeams;    
    }
     
     
    /**  Add an entry to the tableModel
     *
     *
     * @return True if succes else False
     */
    public boolean addRow(Beam aBeam) {
      
        if (aBeam==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }
        
        itsBeams.add(aBeam);

        String[]  newRow = { aBeam.getDirectionType(),
                             aBeam.getAngle1(),
                             aBeam.getAngle2(),
                             aBeam.getCoordType(),
                             aBeam.getNrTiedArrayBeams(),
                             aBeam.getSubbandList(),
                             aBeam.getBeamletList() };
        this.addRow(newRow);
        itsBeams.trimToSize();


        isChanged=true;
        return true;
    }
    
    /** Update a row with new information
     *
     * @param   newBeam that contains the newBeam that hould replace another Beam
     * @param   row     int with the rownumber.
     */
    public boolean updateRow(Beam newBeam,int row) {
        if (row < this.getRowCount() && row >= 0) {
            if (itsBeams != null) {
                itsBeams.set(row+offset, newBeam);
                itsBeams.trimToSize();
            }
        } else {
            logger.error("Error in updateRow, illegal rownumber supplied");
            return false;
        }
        
        
        this.setValueAt(newBeam.getDirectionType(),row,0);
        this.setValueAt(newBeam.getAngle1(),row,1);
        this.setValueAt(newBeam.getAngle2(),row,2);
        this.setValueAt(newBeam.getCoordType(),row,3);
        this.setValueAt(newBeam.getNrTiedArrayBeams(),row,5);
        this.setValueAt(newBeam.getSubbandList(),row,6);
        this.setValueAt(newBeam.getBeamletList(),row,7);
        
        isChanged=true;

        this.fireTableDataChanged();
        return true;
    }
    
    /** get the values from the given row
     *
     * @param   row int with row number
     *
     * @return  String[] containing all values from the given row
     */
    public Beam getSelection(int row) {
        if (row < this.getRowCount() && row >= 0 && itsBeams != null) {
            if (itsBeams != null) {
                return itsBeams.get(row+offset);
            }
        }
        return null;
                           
    }

    public boolean changed() {
        return isChanged;
    }

    public void removeAllRows() {
        this.setRowCount(0);
        if (itsBeams != null) {
            itsBeams.clear();
            itsBeams.trimToSize();
            isChanged=true;
        }
    }

    @Override
    public void removeRow(int row) {
        super.removeRow(row);
        if (itsBeams != null) {
            itsBeams.remove(row+offset);
            itsBeams.trimToSize();
            isChanged=true;
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

    @Override
    public Class getColumnClass(int c) {
        Object value=this.getValueAt(0,c);
        return (value==null?Object.class:value.getClass());
    }
}
