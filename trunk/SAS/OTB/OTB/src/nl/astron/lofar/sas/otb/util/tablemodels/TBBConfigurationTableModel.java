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
    
    
    /** Creates a new instance of TBBConfigurationTableModel */
    public TBBConfigurationTableModel() { 
        this.addColumn("mode");
        this.addColumn("base");
        this.addColumn("start");
        this.addColumn("stop");
        this.addColumn("filter");
        this.addColumn("window");
        this.addColumn("C0");
        this.addColumn("C1");
        this.addColumn("C2");
        this.addColumn("C3");
        this.addColumn("RCUs");
        this.addColumn("Subbands");
        
    }
    
    /** fills the table with the initial settings
     *
     * @param  aMode          Vector<String> of all OperatingModes
     * @param  aBase          Vector<String> of all Baselevels
     * @param  aStart         Vector<String> of all Startlevels
     * @param  aStop          Vector<String> of all Stoplevels
     * @param  aFilter        Vector<String> of all Filters
     * @param  aWindow        Vector<String> of all Windows
     * @param  aC0            Vector<String> of all Coeff0s
     * @param  aC1            Vector<String> of all Coeff1s
     * @param  aC2            Vector<String> of all Coeff2s
     * @param  aC3            Vector<String> of all Coeff3s
     * @param  aRCUs          Vector<String> of all RCUs involved
     * @param  aSubbandList   Vector<String> of all subbands involved
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType,Vector<String> aMode,Vector<String> aBase,Vector<String> aStart, 
                             Vector<String> aStop, Vector<String> aFilter, Vector<String> aWindow, Vector<String> aC0, 
                             Vector<String> aC1, Vector<String> aC2, Vector<String> aC3, Vector<String> aRCUs,
                             Vector<String> aSubbandList) {
         
        itsTreeType=treeType;
        // "clear" the table
        setRowCount(0);
        if (aMode==null||aBase==null||aStart==null||aStop==null||aFilter==null||aWindow==null||aC0==null||aC1==null||
                aC2==null||aC3==null||aRCUs==null) {
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
                                 aBase.elementAt(i+offset),
                                 aStart.elementAt(i+offset),
                                 aStop.elementAt(i+offset),
                                 aFilter.elementAt(i+offset),
                                 aWindow.elementAt(i+offset),
                                 aC0.elementAt(i+offset),
                                 aC1.elementAt(i+offset),
                                 aC2.elementAt(i+offset),
                                 aC3.elementAt(i+offset),
                                 aRCUs.elementAt(i+offset),
                                 aSubbandList.elementAt(i+offset)};
            this.addRow(newRow);
        }
        fireTableDataChanged();
        return true;    
    }
 
    /** fills the table with the initial settings
     *
     * @param  aMode          Vector<String> of all OperatingModes
     * @param  aBase          Vector<String> of all Baselevels
     * @param  aStart         Vector<String> of all Startlevels
     * @param  aStop          Vector<String> of all Stoplevels
     * @param  aFilter        Vector<String> of all Filters
     * @param  aWindow        Vector<String> of all Windows
     * @param  aC0            Vector<String> of all Coeff0s
     * @param  aC1            Vector<String> of all Coeff1s
     * @param  aC2            Vector<String> of all Coeff2s
     * @param  aC3            Vector<String> of all Coeff3s
     * @param  aRCUs          Vector<String> of all RCUs involved
     * @param  aSubbandList   Vector<String> of all subbands involved
     *
     * @return True if succes else False
     */
     public boolean getTable(Vector<String> aMode,Vector<String> aBase,Vector<String> aStart, 
                             Vector<String> aStop, Vector<String> aFilter, Vector<String> aWindow, Vector<String> aC0, 
                             Vector<String> aC1, Vector<String> aC2, Vector<String> aC3, Vector<String> aRCUs,
                             Vector<String> aSubbandList) {
         
        int length = aMode.size();
        
        // need to skip first entry because it is the default (dummy) TBBsetting
        // empty all elements except the default
        aMode.setSize(1);
        aBase.setSize(1);
        aStart.setSize(1);
        aStop.setSize(1);
        aFilter.setSize(1);
        aWindow.setSize(1);
        aC0.setSize(1);
        aC1.setSize(1);
        aC2.setSize(1);
        aC3.setSize(1);
        aRCUs.setSize(1);
        aSubbandList.setSize(1);
        
        
        for (int i=0; i<getRowCount(); i++) {
            aMode.addElement((String)getValueAt(i,0));
            aBase.addElement((String)getValueAt(i,1));
            aStart.addElement((String)getValueAt(i,2));
            aStop.addElement((String)getValueAt(i,3));
            aFilter.addElement((String)getValueAt(i,4));
            aWindow.addElement((String)getValueAt(i,5));
            aC0.addElement((String)getValueAt(i,6));
            aC1.addElement((String)getValueAt(i,7));
            aC2.addElement((String)getValueAt(i,8));
            aC3.addElement((String)getValueAt(i,9));
            aRCUs.addElement((String)getValueAt(i,10));
            aSubbandList.addElement((String)getValueAt(i,11));            
        }
        return true;    
    }
     
     
    /**  Add an entry to the tableModel
     *
     * @param  aMode        String OperatingMode
     * @param  aBase        String Baselevel
     * @param  aStart       String Startlevel
     * @param  aStop        String Stoplevel
     * @param  aFilter      String Filter
     * @param  aWindow      String Window
     * @param  aC0          String Coeff0
     * @param  aC1          String Coeff1
     * @param  aC2          String Coeff2
     * @param  aC3          String Coeff3
     * @param  aRCUs        String RCUs involved
     * @param  aSubbandList String subbands involved
     *
     * @return True if succes else False
     */
    public boolean addRow(String aMode,String aBase,String aStart, String aStop, String aFilter, 
                          String aWindow,String aC0, String aC1, String aC2, String aC3, String aRCUs) {
      
        if (aMode==null||aBase==null||aStart==null||aStop==null||aFilter==null||aWindow==null||aC0==null||aC1==null||
                aC2==null||aC3==null||aRCUs==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }
        String[]  newRow = { aMode,aBase,aStart,aStop,aFilter,aWindow,aC0,aC1,aC2,aC3,aRCUs };
        this.addRow(newRow);
        
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
                                   (String)this.getValueAt(row,11)};
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
