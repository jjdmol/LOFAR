/*
 * ParamExtensionTableModel.java
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
 * Implements the data model behind the paramExtension table
 *
 * @created 03-08-2010, 9:00
 *
 *
 * @version $Id$
 *
 * @updated
 */

package nl.astron.lofar.sas.otb.util.tablemodels;

import java.util.Vector;
import nl.astron.lofar.lofarutils.LofarUtils;
import org.apache.log4j.Logger;

/**
 *
 * @author Coolen
 */
public class ParamExtensionTableModel extends javax.swing.table.DefaultTableModel {
   
    static Logger logger = Logger.getLogger(ParamExtensionTableModel.class);
    static String name = "ParamExtensionTableModel";

    private String itsTreeType = null;

    private Vector<String> itsKeys    = new Vector<String>();
    private Vector<String> itsValues  = new Vector<String>();

    private boolean isChanged=false;

    /** Creates a new instance of ParamExtensionTableModel */
    public ParamExtensionTableModel() {
        this.addColumn("Key");
        this.addColumn("Value");
    }
    
    /** fills the table with the initial settings
     *
     * @param aList       comma seperated list with all K=V pairs
     * @param  refill      false for initial fill, true for refill
     *
     * @return True if succes else False
     */
     public boolean fillTable(String aS,boolean refill){
        // "clear" the table
        setRowCount(0);

        if (aS.equals("")) {
            logger.info("List empty,nothing to do.");
            return true;
        }
        
        //empty old settings
        removeAllRows();

        String[] aList=LofarUtils.arrayFromList(aS);

        for (int i=0;i<aList.length;i++){

            // split "key=val"
            String [] keyval = aList[i].split("=");
            if (keyval.length < 2) {
                continue;
            }
            if (keyval[0]!= null) {
                itsKeys.add(keyval[0].replaceAll("\\s+",""));
            } else {
                itsKeys.add("");
            }
            if (keyval[1]!= null) {
                itsValues.add(keyval[1].replaceAll("\\s+",""));
            } else {
                itsValues.add("");
            }
            String[] newRow = {itsKeys.elementAt(i),itsValues.elementAt(i)};
            this.addRow(newRow);
        }

        
        isChanged=refill;
        fireTableDataChanged();
        return true;    
    }
 
    /** get TableValues
     *
     * @param  anAngles1   Vector<String> of all direction 1 angles
     *
     * @return True if succes else False
     */
     public String getTable() {

         String aList="[";
        for (int i=0; i<getRowCount(); i++) {
            if (i >0) aList+=",";
            aList+="\"";
            aList += (String)getValueAt(i,0);
            aList += "=";
            aList += (String)getValueAt(i,1);
            aList+="\"";
        }
         aList+="]";

        return aList;
    }
     
   /**  Add an entry to the tableModel
     *
     * @param  aString   String with key/value pair
     *
     * @return True if succes else False
     */
    public boolean addRow(String aKey, String aVal) {

        if (aKey.isEmpty() || aVal.isEmpty()) {
            logger.info("empty string, no key/val found");
            return true;
        }

        itsKeys.addElement(aKey);
        itsValues.addElement(aVal);

        String[] newRow = {aKey, aVal};
        this.addRow(newRow);
        isChanged = true;
        return true;
    }

    /** Update a row with new information
     *
     * @param   newRow  String[] that contains all values as they should be for this row
     * @param   row     int with the rownumber.
     */
    public boolean updateRow(String[] newRow,int row) {
        if (row < this.getRowCount() && row >= 0) {
            //Key
            this.setValueAt(newRow[0],row,0);
            itsKeys.setElementAt(newRow[0], row);
            //Value
            this.setValueAt(newRow[1],row,1);
            itsValues.setElementAt(newRow[1], row);
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
                                   (String)this.getValueAt(row,1)};
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
        itsKeys.removeAllElements();
        itsValues.removeAllElements();
        isChanged=true;
    }

    @Override
    public void removeRow(int row) {
        super.removeRow(row);
        itsKeys.remove(row);
        itsValues.remove(row);
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
