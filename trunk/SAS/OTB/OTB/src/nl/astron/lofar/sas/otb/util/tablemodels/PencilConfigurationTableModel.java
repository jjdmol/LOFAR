/*
 * PencilConfigurationTableModel.java
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
 * Implements the data model behind the PencilConfiguration table
 *
 * @created 11-02-2010
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
public class PencilConfigurationTableModel extends javax.swing.table.DefaultTableModel {

    static Logger logger = Logger.getLogger(PencilConfigurationTableModel.class);
    static String name = "PencilConfigurationTableModel";

    private String itsTreeType = null;

    private Vector<String> itsAngles1    = new Vector<String>();
    private Vector<String> itsAngles2    = new Vector<String>();
    private Vector<String> itsCoordTypes = new Vector<String>();

    private int offset=1;

    private boolean isChanged=false;

    /** Creates a new instance of PencilConfigurationTableModel */
    public PencilConfigurationTableModel() {
        this.addColumn("angle 1");
        this.addColumn("angle 2");
        this.addColumn("coordtype");
    }

    /** fills the table with the initial settings
     *
     * @param  anAngles1   Vector<String> of all direction 1 angles
     * @param  anAngels2   Vector<String> of all direction 2 angles
     * @param  refill     false for initial fill, true for refill
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType, Vector<String> anAngles1,Vector<String> anAngles2,Vector<String> aCoordType,boolean refill){
        // "clear" the table
        setRowCount(0);

        if (anAngles1==null||anAngles2==null|| aCoordType==null) {
            logger.error("Error in fillTable, null value in input found.");
            return false;
        }
        itsTreeType=treeType;
        int length = anAngles1.size();

        //empty old settings
        removeAllRows();


        for (int i=0;i<length;i++) {
            itsAngles1.add(anAngles1.get(i));
            itsAngles2.add(anAngles2.get(i));
            itsCoordTypes.add(aCoordType.get(i));
        }

        // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }

        // need to skip first entry because it is the default (dummy) TBBsetting
        for (int i=0; i<length-offset; i++) {
            String[]  newRow = { itsAngles1.elementAt(i+offset),
                                 itsAngles2.elementAt(i+offset),
                                 itsCoordTypes.elementAt(i+offset),
 };

            this.addRow(newRow);
        }
        isChanged=refill;
        fireTableDataChanged();
        return true;
    }

    /** fills the table with the initial settings
     *
     * @param  anAngles1   Vector<String> of all direction 1 angles
     * @param  anAngels2   Vector<String> of all direction 2 angles
     * @param  aCoordType  Vector<String> of all coordinate types
     *
     * @return True if succes else False
     */
     public boolean getTable(Vector<String> anAngles1,Vector<String> anAngles2,Vector<String> aCoordType) {

        // need to skip first entry because it is the default (dummy) TBBsetting
        // empty all elements except the default
        anAngles1.setSize(1);
        anAngles2.setSize(1);
        aCoordType.setSize(1);

        for (int i=0; i<getRowCount(); i++) {
            anAngles1.addElement((String)getValueAt(i,0));
            anAngles2.addElement((String)getValueAt(i,1));
            aCoordType.addElement((String)getValueAt(i,2));
        }
        return true;
    }


    /**  Add an entry to the tableModel
     *
     * @param  anAngles1   direction 1 angle
     * @param  anAngels2   direction 2 angle
     * @param  aCoordType  coordinate types involved
     *
     * @return True if succes else False
     */
    public boolean addRow(String anAngle1,String anAngle2, String aCoordType) {

        if (anAngle1==null||anAngle2==null || aCoordType==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }
        itsAngles1.add(anAngle1);
        itsAngles2.add(anAngle2);
        itsCoordTypes.add(aCoordType);

        String[]  newRow = {anAngle1,anAngle2,aCoordType};
        this.addRow(newRow);

        isChanged=true;
        return true;
    }

    /** Update a row with new information
     *
     * @param   newRow  String[] that contains all values as they should be for this row
     * @param   row     int with the rownumber.
     */
    public boolean updateRow(String[] newRow,int row) {
        if (row < this.getRowCount() && row >= 0) {
            //Angle1
            this.setValueAt(newRow[0],row,0);
            itsAngles1.setElementAt(newRow[0], row+offset);
            //Angle2
            this.setValueAt(newRow[1],row,1);
            itsAngles2.setElementAt(newRow[1], row+offset);
            //CoordType
            this.setValueAt(newRow[2],row,2);
            itsCoordTypes.setElementAt(newRow[2], row+offset);

        } else {
            logger.error("Error in updateRow, illegal rownumber supplied");
            return false;
        }
        isChanged=true;

        fireTableDataChanged();
        return true;
    }

    /** get the values from the given row
     *
     * @param   row int with row number
     *
     * @return  String[] containing all values from the given row
     */
    public String[] getSelection(int row) {
        if (row < this.getRowCount() && row >= 0) {
            String[] selection = { (String)this.getValueAt(row,0),
                                   (String)this.getValueAt(row,1),
                                   (String)this.getValueAt(row,2)};
            return selection;
        } else {
            return null;
        }
    }

    public boolean changed() {
        return isChanged;
    }

    public void removeAllRows() {
        this.setRowCount(0);
        itsAngles1.removeAllElements();
        itsAngles2.removeAllElements();
        itsCoordTypes.removeAllElements();

        isChanged=true;
    }

    @Override
    public void removeRow(int row) {
        super.removeRow(row);
        itsAngles1.remove(row+offset);
        itsAngles2.remove(row+offset);
        itsCoordTypes.remove(row+offset);
        isChanged=true;
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
