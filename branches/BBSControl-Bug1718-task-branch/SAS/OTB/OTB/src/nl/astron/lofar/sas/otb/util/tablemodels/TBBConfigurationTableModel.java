/*
 * TBBConfigurationTableModel.java
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
 * Implements the data model behind the TBBConfiguration table
 *
 * @created 07-06-2006, 13:30
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
public class TBBConfigurationTableModel extends javax.swing.table.DefaultTableModel {
   
    static Logger logger = Logger.getLogger(TBBConfigurationTableModel.class);
    static String name = "TBBConfigurationTableModel";
    
    private String itsTreeType;

    private boolean isChanged=false;
    
    
    /** Creates a new instance of TBBConfigurationTableModel */
    public TBBConfigurationTableModel() { 
        this.addColumn("mode");
        this.addColumn("trigger");
        this.addColumn("base");
        this.addColumn("start");
        this.addColumn("stop");
        this.addColumn("filter");
        this.addColumn("window");
        this.addColumn("F0C0");
        this.addColumn("F0C1");
        this.addColumn("F0C2");
        this.addColumn("F0C3");
        this.addColumn("F1C0");
        this.addColumn("F1C1");
        this.addColumn("F1C2");
        this.addColumn("F1C3");
        this.addColumn("RCUs");
        this.addColumn("Subbands");
        
    }
    
    /** fills the table with the initial settings
     *
     * @param  aMode          Vector<String> of all OperatingModes
     * @param  aTrigger       Vector<String> of all TriggerModes
     * @param  aBase          Vector<String> of all Baselevels
     * @param  aStart         Vector<String> of all Startlevels
     * @param  aStop          Vector<String> of all Stoplevels
     * @param  aFilter        Vector<String> of all Filters
     * @param  aWindow        Vector<String> of all Windows
     * @param  aF0C0            Vector<String> of all Filter0Coeff0s
     * @param  aF0C1            Vector<String> of all Filter0Coeff1s
     * @param  aF0C2            Vector<String> of all Filter0Coeff2s
     * @param  aF0C3            Vector<String> of all Filter0Coeff3s
     * @param  aF1C0            Vector<String> of all Filter1Coeff0s
     * @param  aF1C1            Vector<String> of all Filter1Coeff1s
     * @param  aF1C2            Vector<String> of all Filter1Coeff2s
     * @param  aF1C3            Vector<String> of all Filter1Coeff3s
     * @param  aRCUs          Vector<String> of all RCUs involved
     * @param  aSubbandList   Vector<String> of all subbands involved
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType,Vector<String> aMode,Vector<String> aTrigger,Vector<String> aBase,Vector<String> aStart,
                             Vector<String> aStop, Vector<String> aFilter, Vector<String> aWindow, Vector<String> aF0C0,
                             Vector<String> aF0C1, Vector<String> aF0C2, Vector<String> aF0C3, Vector<String> aF1C0,
                             Vector<String> aF1C1, Vector<String> aF1C2, Vector<String> aF1C3, Vector<String> aRCUs,
                             Vector<String> aSubbandList) {
         
        itsTreeType=treeType;
        // "clear" the table
        setRowCount(0);
        if (aMode==null||aTrigger==null||aBase==null||aStart==null||aStop==null||aFilter==null||aWindow==null||aF0C0==null||
                aF0C1==null||aF0C2==null||aF0C3==null||aF1C0==null||aF1C1==null||aF1C2==null||aF1C3==null||aRCUs==null) {
            logger.error("Error in fillTable, null value in input found.");
            return false;
        }
        int length = aMode.size();
        
        // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        int offset=1;
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }
        for (int i=0; i<length-offset; i++) {
            String[]  newRow = { aMode.elementAt(i+offset),
                                 aTrigger.elementAt(i+offset),
                                 aBase.elementAt(i+offset),
                                 aStart.elementAt(i+offset),
                                 aStop.elementAt(i+offset),
                                 aFilter.elementAt(i+offset),
                                 aWindow.elementAt(i+offset),
                                 aF0C0.elementAt(i+offset),
                                 aF0C1.elementAt(i+offset),
                                 aF0C2.elementAt(i+offset),
                                 aF0C3.elementAt(i+offset),
                                 aF1C0.elementAt(i+offset),
                                 aF1C1.elementAt(i+offset),
                                 aF1C2.elementAt(i+offset),
                                 aF1C3.elementAt(i+offset),
                                 aRCUs.elementAt(i+offset),
                                 aSubbandList.elementAt(i+offset)};
            this.addRow(newRow);
        }
        // only initial settings added
        isChanged=false;
        fireTableDataChanged();
        return true;    
    }
 
    /** fills the table with the initial settings
     *
     * @param  aMode          Vector<String> of all OperatingModes
     * @param  aTrigger       Vector<String> of all triggerModes
     * @param  aBase          Vector<String> of all Baselevels
     * @param  aStart         Vector<String> of all Startlevels
     * @param  aStop          Vector<String> of all Stoplevels
     * @param  aFilter        Vector<String> of all Filters
     * @param  aWindow        Vector<String> of all Windows
     * @param  aF0C0            Vector<String> of all Filter0Coeff0s
     * @param  aF0C1            Vector<String> of all Filter0Coeff1s
     * @param  aF0C2            Vector<String> of all Filter0Coeff2s
     * @param  aF0C3            Vector<String> of all Filter0Coeff3s
     * @param  aF1C0            Vector<String> of all Filter1Coeff0s
     * @param  aF1C1            Vector<String> of all Filter1Coeff1s
     * @param  aF1C2            Vector<String> of all Filter1Coeff2s
     * @param  aF1C3            Vector<String> of all Filter1Coeff3s
     * @param  aRCUs          Vector<String> of all RCUs involved
     * @param  aSubbandList   Vector<String> of all subbands involved
     *
     * @return True if succes else False
     */
     public boolean getTable(Vector<String> aMode,Vector<String> aTrigger,Vector<String> aBase,Vector<String> aStart,
                             Vector<String> aStop, Vector<String> aFilter, Vector<String> aWindow, Vector<String> aF0C0,
                             Vector<String> aF0C1, Vector<String> aF0C2, Vector<String> aF0C3, Vector<String> aF1C0,
                             Vector<String> aF1C1, Vector<String> aF1C2, Vector<String> aF1C3, Vector<String> aRCUs,
                             Vector<String> aSubbandList) {
         
        int length = aMode.size();
        
        // need to skip first entry because it is the default (dummy) TBBsetting
        // empty all elements except the default
        aMode.setSize(1);
        aTrigger.setSize(1);
        aBase.setSize(1);
        aStart.setSize(1);
        aStop.setSize(1);
        aFilter.setSize(1);
        aWindow.setSize(1);
        aF0C0.setSize(1);
        aF0C1.setSize(1);
        aF0C2.setSize(1);
        aF0C3.setSize(1);
        aF1C0.setSize(1);
        aF1C1.setSize(1);
        aF1C2.setSize(1);
        aF1C3.setSize(1);
        aRCUs.setSize(1);
        aSubbandList.setSize(1);
        
        
        for (int i=0; i<getRowCount(); i++) {
            aMode.addElement((String)getValueAt(i,0));
            aTrigger.addElement((String)getValueAt(i,1));
            aBase.addElement((String)getValueAt(i,2));
            aStart.addElement((String)getValueAt(i,3));
            aStop.addElement((String)getValueAt(i,4));
            aFilter.addElement((String)getValueAt(i,5));
            aWindow.addElement((String)getValueAt(i,6));
            aF0C0.addElement((String)getValueAt(i,7));
            aF0C1.addElement((String)getValueAt(i,8));
            aF0C2.addElement((String)getValueAt(i,9));
            aF0C3.addElement((String)getValueAt(i,10));
            aF1C0.addElement((String)getValueAt(i,11));
            aF1C1.addElement((String)getValueAt(i,12));
            aF1C2.addElement((String)getValueAt(i,13));
            aF1C3.addElement((String)getValueAt(i,14));
            aRCUs.addElement((String)getValueAt(i,15));
            aSubbandList.addElement((String)getValueAt(i,16));
        }
        return true;    
    }
     
     
    /**  Add an entry to the tableModel
     *
     * @param  aMode        String OperatingMode
     * @param  aTrigger     String TriggerMode
     * @param  aBase        String Baselevel
     * @param  aStart       String Startlevel
     * @param  aStop        String Stoplevel
     * @param  aFilter      String Filter
     * @param  aWindow      String Window
     * @param  aF0C0          String Filter0Coeff0
     * @param  aF0C1          String Filter0Coeff1
     * @param  aF0C2          String Filter0Coeff2
     * @param  aF0C3          String Filter0Coeff3
     * @param  aF1C0          String Filter1Coeff0
     * @param  aF1C1          String Filter1Coeff1
     * @param  aF1C2          String Filter1Coeff2
     * @param  aF1C3          String Filter1Coeff3
     * @param  aRCUs        String RCUs involved
     * @param  aSubbandList String subbands involved
     *
     * @return True if succes else False
     */
    public boolean addRow(String aMode,String aTrigger,String aBase,String aStart, String aStop, String aFilter,
                          String aWindow,String aF0C0, String aF0C1, String aF0C2,String aF0C3, String aF1C0,
                          String aF1C1,String aF1C2, String aF1C3, String aRCUs, String aSBList) {
      
        if (aMode==null||aTrigger==null||aBase==null||aStart==null||aStop==null||aFilter==null||aWindow==null|| aF0C0==null||aF0C1==null||aF0C2==null||aF0C3==null||aF1C0==null||aF1C1==null||aF1C2==null||aF1C3==null||aRCUs==null||aSBList==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }
        String[]  newRow = { aMode,aTrigger,aBase,aStart,aStop,aFilter,aWindow,aF0C0,aF0C1,aF0C2,aF0C3,aF1C0,aF1C1,aF1C2,aF1C3,aRCUs,aSBList };
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
            this.setValueAt(newRow[0],row,0);
            this.setValueAt(newRow[1],row,1);
            this.setValueAt(newRow[2],row,2);
            this.setValueAt(newRow[3],row,3);
            this.setValueAt(newRow[4],row,4);
            this.setValueAt(newRow[5],row,5);
            this.setValueAt(newRow[6],row,6);
            this.setValueAt(newRow[7],row,7);
            this.setValueAt(newRow[8],row,8);
            this.setValueAt(newRow[9],row,9);
            this.setValueAt(newRow[10],row,10);
            this.setValueAt(newRow[11],row,11);
            this.setValueAt(newRow[12],row,12);
            this.setValueAt(newRow[13],row,13);
            this.setValueAt(newRow[14],row,14);
            this.setValueAt(newRow[15],row,15);
            this.setValueAt(newRow[16],row,16);
        } else {
            logger.error("Error in updateRow, illegal rownumber supplied: "+row);
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
                                   (String)this.getValueAt(row,2),
                                   (String)this.getValueAt(row,3),
                                   (String)this.getValueAt(row,4),
                                   (String)this.getValueAt(row,5),
                                   (String)this.getValueAt(row,6),
                                   (String)this.getValueAt(row,7),
                                   (String)this.getValueAt(row,8),
                                   (String)this.getValueAt(row,9),
                                   (String)this.getValueAt(row,10),
                                   (String)this.getValueAt(row,11),
                                   (String)this.getValueAt(row,12),
                                   (String)this.getValueAt(row,13),
                                   (String)this.getValueAt(row,14),
                                   (String)this.getValueAt(row,15),
                                   (String)this.getValueAt(row,16)};
            return selection;
        } else {
            return null;
        }
                               
    }

    @Override
    public void addRow(Object[] newRow) {
        isChanged=true;
        super.addRow(newRow);
    }

    @Override
    public void removeRow(int row) {
        isChanged=true;
        super.removeRow(row);
    }

    public boolean changed() {
        return isChanged;
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
