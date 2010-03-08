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

import java.util.Vector;
import org.apache.log4j.Logger;

/**
 *
 * @author Coolen
 */
public class BeamConfigurationTableModel extends javax.swing.table.DefaultTableModel {
   
    static Logger logger = Logger.getLogger(BeamConfigurationTableModel.class);
    static String name = "BeamConfigurationTableModel";

    private String itsTreeType = null;

    private Vector<String> itsDirTypes   = new Vector<String>();
    private Vector<String> itsAngles1    = new Vector<String>();
    private Vector<String> itsAngles2    = new Vector<String>();
    private Vector<String> itsDurations  = new Vector<String>();
    private Vector<String> itsStartTimes = new Vector<String>();
    private Vector<String> itsSubbands   = new Vector<String>();
    private Vector<String> itsBeamlets   = new Vector<String>();
    private Vector<String> itsMomIDs     = new Vector<String>();

    private int offset=1;

    /** Creates a new instance of BeamConfigurationTableModel */
    public BeamConfigurationTableModel() { 
        this.addColumn("dirtype");
        this.addColumn("angle 1");
        this.addColumn("angle 2");
        this.addColumn("subbands");
        this.addColumn("beamlets");
    }
    
    /** fills the table with the initial settings
     *
     * @param  aDirTypes   Vector<String> of all direction Types
     * @param  anAngles1   Vector<String> of all direction 1 angles
     * @param  anAngels2   Vector<String> of all direction 2 angles
     * @param  durations   Vector<String> of all durations
     * @param  startTimes  Vector<String> of all startTimes
     * @param  aSubbands   Vector<String> of all Subbands involved
     * @param  aBeamlets   Vector<String> of all Beamlets involved
     * @param  aMomIDs     Vector<String> of all momIDs
     *
     * @return True if succes else False
     */
     public boolean fillTable(String treeType, Vector<String> aDirTypes,Vector<String> anAngles1,Vector<String> anAngles2, 
                             Vector<String> aDurations,Vector<String> aStartTimes,Vector<String> aSubbands,
                             Vector<String> aBeamlets,Vector<String> aMomIDs){
        // "clear" the table
        setRowCount(0);

        if (aDirTypes==null||anAngles1==null||anAngles2==null||aDurations==null||aStartTimes==null||
                aSubbands==null||aBeamlets==null||aMomIDs==null) {
            logger.error("Error in fillTable, null value in input found.");
            return false;
        }
        itsTreeType=treeType;
        int length = aDirTypes.size();
        
        //empty old settings
        this.setRowCount(0);
        itsDirTypes.removeAllElements();
        itsAngles1.removeAllElements();
        itsAngles2.removeAllElements();
        itsDurations.removeAllElements();
        itsStartTimes.removeAllElements();
        itsSubbands.removeAllElements();
        itsBeamlets.removeAllElements();
        itsMomIDs.removeAllElements();


        for (int i=0;i<length;i++) {
            itsDirTypes.add(aDirTypes.get(i));
            itsAngles1.add(anAngles1.get(i));
            itsAngles2.add(anAngles2.get(i));
            itsDurations.add(aDurations.get(i));
            itsStartTimes.add(aStartTimes.get(i));
            itsSubbands.add(aSubbands.get(i));
            itsBeamlets.add(aBeamlets.get(i));
            itsMomIDs.add(aMomIDs.get(i));
        }
        
        // need to skip first entry because it is the default (dummy) TBBsetting in other then VHTree's
        if (itsTreeType.equals("VHtree")) {
            offset=0;
        }
        
        // need to skip first entry because it is the default (dummy) TBBsetting
        for (int i=0; i<length-offset; i++) {
            String[]  newRow = { itsDirTypes.elementAt(i+offset),
                                 itsAngles1.elementAt(i+offset),
                                 itsAngles2.elementAt(i+offset),
                                 itsSubbands.elementAt(i+offset),
                                 itsBeamlets.elementAt(i+offset)};
            
            this.addRow(newRow);
        }
        fireTableDataChanged();
        return true;    
    }
 
    /** fills the table with the initial settings
     *
     * @param  aDirTypes   Vector<String> of all direction Types
     * @param  anAngles1   Vector<String> of all direction 1 angles
     * @param  anAngels2   Vector<String> of all direction 2 angles
     * @param  aDurations  Vector<String> of all direction 2 angles
     * @param  aStartTimes Vector<String> of all direction 2 angles
     * @param  aSubbands   Vector<String> of all Subbands involved
     * @param  aBeamlets   Vector<String> of all Beamlets involved
     * @param  aMomIDs     Vector<String> of all direction 2 angles
     *
     * @return True if succes else False
     */
     public boolean getTable(Vector<String> aDirTypes,Vector<String> anAngles1,Vector<String> anAngles2, 
                             Vector<String> aDurations,Vector<String> aStartTimes,Vector<String> aSubbands,
                             Vector<String> aBeamlets,Vector<String> aMomIDs) {

        // need to skip first entry because it is the default (dummy) TBBsetting
        // empty all elements except the default
        aDirTypes.setSize(1);
        anAngles1.setSize(1);
        anAngles2.setSize(1);
        aDurations.setSize(1);
        aStartTimes.setSize(1);
        aSubbands.setSize(1);
        aBeamlets.setSize(1);
        aMomIDs.setSize(1);

        for (int i=0; i<getRowCount(); i++) {
            aDirTypes.addElement((String)getValueAt(i,0));
            anAngles1.addElement((String)getValueAt(i,1));
            anAngles2.addElement((String)getValueAt(i,2));
            aDurations.addElement(itsDurations.get(i+offset));
            aStartTimes.addElement(itsStartTimes.get(i+offset));
            aSubbands.addElement((String)getValueAt(i,3));
            aBeamlets.addElement((String)getValueAt(i,4));
            aMomIDs.addElement(itsMomIDs.get(i+offset));
        }
        return true;    
    }
     
     
    /**  Add an entry to the tableModel
     *
     * @param  aDirTypes   direction Type
     * @param  anAngles1   direction 1 angle
     * @param  anAngels2   direction 2 angle
     * @param  aDuration   duration involved
     * @param  aStartTime  startTime involved
     * @param  aSubbands   Subbands involved
     * @param  aBeamlets   Beamlets involved
     * @param  aMomID      momID involved
     *
     * @return True if succes else False
     */
    public boolean addRow(String aDirType,String anAngle1,String anAngle2, String aDuration, String aStartTime,
            String aSubbands, String aBeamlets, String aMomID) {
      
        if (aDirType==null||anAngle1==null||anAngle2==null||aDuration==null||aStartTime==null||
                aSubbands==null||aBeamlets==null||aMomID==null) {
            logger.error("Error in addRow, null value in input found.");
            return false;
        }
        itsDirTypes.add(aDirType);
        itsAngles1.add(anAngle1);
        itsAngles2.add(anAngle2);
        itsDurations.add(aDuration);
        itsStartTimes.add(aStartTime);
        itsSubbands.add(aSubbands);
        itsBeamlets.add(aBeamlets);
        itsMomIDs.add(aMomID);

        String[]  newRow = { aDirType,anAngle1,anAngle2,aSubbands,aBeamlets};
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
            //DirType
            this.setValueAt(newRow[0],row,0);
            itsDirTypes.setElementAt(newRow[0], row+offset);
            //Angle1
            this.setValueAt(newRow[1],row,1);
            itsAngles1.setElementAt(newRow[1], row+offset);
            //Angle2
            this.setValueAt(newRow[2],row,2);
            itsAngles2.setElementAt(newRow[2], row+offset);
            //Duration
            itsDurations.setElementAt(newRow[3], row+offset);
            //StartTime
            itsStartTimes.setElementAt(newRow[4], row+offset);
            //Subband
            this.setValueAt(newRow[5],row,3);
            itsSubbands.setElementAt(newRow[5], row+offset);
            //Beamlet
            this.setValueAt(newRow[6],row,4);
            itsBeamlets.setElementAt(newRow[6], row+offset);
            //MomID
            itsMomIDs.setElementAt(newRow[7], row+offset);
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
                                   itsDurations.get(row+offset),
                                   itsStartTimes.get(row+offset),
                                   (String)this.getValueAt(row,3),
                                   (String)this.getValueAt(row,4),
                                   itsMomIDs.get(row+offset)};
            return selection;
        } else {
            return null;
        }                           
    }





    @Override
    public void removeRow(int row) {
        super.removeRow(row);
        itsDirTypes.remove(row+offset);
        itsAngles1.remove(row+offset);
        itsAngles2.remove(row+offset);
        itsDurations.remove(row+offset);
        itsStartTimes.remove(row+offset);
        itsSubbands.remove(row+offset);
        itsBeamlets.remove(row+offset);
        itsMomIDs.remove(row+offset);
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
